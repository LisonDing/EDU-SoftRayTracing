#pragma once

#include "rtweekend/hittable.hpp"
#include "rtweekend/ray.hpp"
#include "rtweekend/vec3.hpp"
#include <cmath>

namespace rt {

struct hit_record; // 前置声明：告诉编译器后面会有个叫 hit_record 的东西


class material {
public:
    virtual ~material() = default;

    virtual bool scatter(
        // 入射光线 r_in，撞击点信息 rec
        // attenuation 衰减系数 本质是反照率，即物体吸收反射光线的比率
        // 散射后的新光线 scattered
        // 散射返回ture
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const =0;

};

class lambertian : public material {
public:
    color albedo;
    // 引入反照率 albedo 定义物体的反射比率
    lambertian(const color& albedo) : albedo(albedo) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        auto scatter_direction = rec.normal + random_in_unit_sphere();

        // Catch degenerate scatter direction
        // 边缘情况处理：如果随机向量刚好和法线相反，相加会变成零向量
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p,scatter_direction,r_in.time());
        attenuation = albedo;
        return true;
    }
};

class metal : public material {
public:
    color albedo;
    // Fuzzed Reflection（模糊反射） 模拟金属面的粗糙度
    double fuzz;

    metal (const color& albedo,double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
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

};

// dielectric 电介质
class dielectric : public material {
public:
    // 接收材质折射率
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
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

}


