#pragma once

#include "rtweekend/hittable.hpp"
#include "rtweekend/ray.hpp"
#include "rtweekend/rtweekend.hpp"
#include "rtweekend/vec3.hpp"
#include "rtweekend/texture.hpp"
#include <cmath>

namespace rt {

struct hit_record; // 前置声明：告诉编译器后面会有个叫 hit_record 的东西


class material {
public:
    virtual ~material() = default;

    virtual color emitted(double u, double v, const point3& p) const {
        return color(0, 0, 0); // 默认不发光
    }

    // virtual bool scatter(
    //     // 入射光线 r_in，撞击点信息 rec
    //     // attenuation 衰减系数 本质是反照率，即物体吸收反射光线的比率
    //     // 散射后的新光线 scattered
    //     // 散射返回ture
    //     const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    // ) const =0;

    // --- 修改：新增了 double& pdf 参数 ---
    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf
    ) const {
        return false;
    }

    // --- 新增：计算材质本身的物理散射分布值 (即分子中的一部分) ---
    virtual double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        return 0;
    }

};

class lambertian : public material {
// public:
//     color albedo;
//     // 引入反照率 albedo 定义物体的反射比率
//     lambertian(const color& albedo) : albedo(albedo) {}

//     bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
//     const override {
//         auto scatter_direction = rec.normal + random_in_unit_sphere();

//         // Catch degenerate scatter direction
//         // 边缘情况处理：如果随机向量刚好和法线相反，相加会变成零向量
//         if (scatter_direction.near_zero())
//             scatter_direction = rec.normal;

//         scattered = ray(rec.p,scatter_direction,r_in.time());
//         attenuation = albedo;
//         return true;
//     }
// };
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf)
    const override {
        // Implementation for Lambertian scattering
        auto scatter_direction = rec.normal + random_in_unit_sphere();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);

        // 计算散射概率密度函数 (PDF) 的值，Lambertian 的 PDF 是 cosθ/π，其中 θ 是散射方向与法线的夹角
        auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
        pdf = cos_theta < 0 ? 0 : cos_theta / pi;
        
        return true;
    }

    double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const override {
        // 朗伯漫反射的物理散射分布也是 cos(theta) / pi
        auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
        return cos_theta < 0 ? 0 : cos_theta / pi;
    }

private:
    // shared_ptr<texture> tex; // 反照率纹理，可以是纯色或更复杂的纹理
    shared_ptr<texture> albedo;
};

class metal : public material {
public:
    color albedo;
    // Fuzzed Reflection（模糊反射） 模拟金属面的粗糙度
    double fuzz;

    metal (const color& albedo,double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf)
    const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);

        // 即由标准反射项 + 随机系数
        // fuzz即对随机单位球的缩放  fuzz大小决定随机量：光线的弹射角度的随机值即粗糙度
        reflected = unit_vector(reflected) + (fuzz * random_in_unit_sphere());

        scattered = ray(rec.p,reflected,r_in.time());
        attenuation = albedo;

        // 判断散射方向和法线点乘，即当类掠射角时，单位球在物体内，判定被吸收
        return (dot(scattered.direction(), rec.normal) > 0);
    }

    double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const override {
        // 金属材质的散射分布是各向同性的，即在所有方向上概率相等
        return 0.5 / pi; // 假设金属的散射是均匀的
    }

};

// dielectric 电介质
class dielectric : public material {
public:
    // 接收材质折射率
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf)
    const override {
        attenuation = color (1.0,1.0,1.0);
        // rec.front_face：true=光线从空气射入材质（外→内）；false=光线从材质射出到空气（内→外）
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        // 单位化入射光线方向——保证后续点积、长度计算的正确性（光追中光线向量必须单位化）
        vec3 unit_direction = unit_vector(r_in.direction());
        // 计算入射角的余弦值（θ₁），并限制范围避免浮点精度问题
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        // 计算入射角的正弦值（sinθ₁），用于全反射判断
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
        //判断是否发生全反射（无法折射），由Snell’s law:η₁·sinθ₁ = η₂·sinθ₂ → sinθ₂ = ri·sinθ₁ , 若sinθ₂>1.0，折射角无实数解，必然全反射
        bool cannot_refract = ri * sin_theta > 1.0;

        // 最终散射光线的方向
        vec3 direction;

        // 全反射或Schlick近似计算的反射概率 > 随机数 → 反射
        if (cannot_refract || reflectance(cos_theta, ri) > random_double())  
            direction = reflect(unit_direction, rec.normal);
        // 非全反射，且反射概率 ≤ 随机数 → 折射
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, direction,r_in.time());
        pdf = 0.5 / pi; // 假设电介质的散射是均匀的
        return true;
    }
private:
    // 材质绝对折射率,相对于真空/空气，空气折射率≈1.0）
    double refraction_index;
    // Schlick 近似计算反射系数（反射概率）
    static double reflectance(double cosine, double ref_idx) {
        // 完整菲涅尔方程 (The Full Fresnel Equations) 需要考虑偏振 (Polarization)
        // R0​：垂直入射（θ=0°）时的反射系数（固定值，由两种介质的折射率决定）
        auto r0 = (1-ref_idx) / (1+ref_idx);
        // *此步骤是为了计算电场振幅比率，光追计算光子能量，其等于振幅的平方，保证了能量永远为正值
        r0 = r0*r0;
        // Schlick Approximation 公式
        return r0 + (1-r0)*pow((1 - cosine),5);
    }
};

// diffuse_light：漫反射光源材质，直接发光，不进行散射
class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color& emit) : tex(make_shared<solid_color>(emit)) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf) const override {
        return false; // 不发生任何反射散射
    }

    color emitted(double u, double v, const point3& p) const {
        return tex->value(u, v, p);
    }

private:
    shared_ptr<texture> tex;
};

// isotropic：各向同性材质，散射方向完全随机，适用于体积散射（如雾、烟等）
class isotropic : public material {
public:
    isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf)
    const override {
        // 在雾气/烟尘内部散射时，方向是绝对随机的球形分布
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        pdf = 0.5 / pi; // 假设各向同性材质的散射是均匀的
        return true;
    }

private:
    shared_ptr<texture> albedo; // 雾气的颜色
};

}


