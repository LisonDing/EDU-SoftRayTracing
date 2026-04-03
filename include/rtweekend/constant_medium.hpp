// 恒定密度介质（Constant Medium）是一个特殊的体积对象，模拟了一个均匀的、充满悬浮颗粒的介质（比如雾、烟）。当光线穿过这个介质时，会有一定概率与其中的颗粒发生碰撞，导致散射。这个类实现了这种效果。

#pragma once

#include "rtweekend.hpp"
#include "hittable.hpp"
#include "material.hpp"
#include "texture.hpp"

namespace rt {

class constant_medium : public hittable {
public:
    // 构造函数：需要一个边界形状(boundary)、密度(d) 和 纹理(a)
    constant_medium(shared_ptr<hittable> b, double d, shared_ptr<texture> a)
      : boundary(b),
        neg_inv_density(-1.0 / d), // 预计算密度的倒数，用于后续概率计算
        phase_function(make_shared<isotropic>(a))
    {}

    constant_medium(shared_ptr<hittable> b, double d, color c)
      : boundary(b),
        neg_inv_density(-1.0 / d),
        phase_function(make_shared<isotropic>(c))
    {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Print occasional samples when debugging. To enable, set enableDebug true.
        const bool enableDebug = false;
        const bool debugging = enableDebug && random_double() < 0.00001;

        hit_record rec1, rec2;

        // 1. 寻找光线第一次撞击边界的时间 (进入点)
        if (!boundary->hit(r, interval::universe, rec1))
            return false;

        // 2. 寻找光线第二次撞击边界的时间 (离开点)
        if (!boundary->hit(r, interval(rec1.t + 0.0001, infinity), rec2))
            return false;

        if (debugging) std::clog << "\nt_min=" << rec1.t << ", t_max=" << rec2.t << '\n';

        // 限制在当前的有效渲染区间内
        if (rec1.t < ray_t.min) rec1.t = ray_t.min;
        if (rec2.t > ray_t.max) rec2.t = ray_t.max;

        if (rec1.t >= rec2.t)
            return false;

        if (rec1.t < 0)
            rec1.t = 0;

        // 光线在介质内部穿行的真实物理长度
        auto ray_length = r.direction().length();
        auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
        
        // 核心数学魔法：比尔-朗伯定律 (Beer-Lambert Law) 的随机采样
        // 算出在这段距离内，光线到底能飞多远才撞上一个悬浮颗粒
        auto hit_distance = neg_inv_density * std::log(random_double());

        // 如果算出的碰撞距离大于光线在边界内飞行的距离，说明光线侥幸穿过了介质，没撞上颗粒！
        if (hit_distance > distance_inside_boundary)
            return false;

        // 走到这里，说明光线在介质内部发生了碰撞（散射）！
        rec.t = rec1.t + hit_distance / ray_length;
        rec.p = r.at(rec.t);

        if (debugging) {
            std::clog << "hit_distance = " <<  hit_distance << '\n'
                      << "rec.t = " <<  rec.t << '\n'
                      << "rec.p = " <<  rec.p << '\n';
        }

        // 对于各向同性材质，法线毫无意义，随便给一个就行
        rec.normal = vec3(1,0,0);  
        rec.front_face = true;     // 也毫无意义
        rec.mat = phase_function;

        return true;
    }

    aabb bounding_box() const override {
        // 体积的包围盒就是边界物体的包围盒
        return boundary->bounding_box();
    }

private:
    shared_ptr<hittable> boundary; // 体积的边界（比如一个盒子或一个球）
    double neg_inv_density;        // -1 / density
    shared_ptr<material> phase_function; // 决定散射方向和颜色的材质
};

} // namespace rt