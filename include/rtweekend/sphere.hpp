// 封装具体的几何逻辑，即hittable的实现

#pragma once

#include "rtweekend/hittable.hpp"
#include "rtweekend/aabb.hpp"
#include "rtweekend/interval.hpp"
#include "rtweekend/material.hpp"
#include "rtweekend/ray.hpp"
#include "rtweekend/rtweekend.hpp"
#include "vec3.hpp"
#include <cmath>
#include <memory>

namespace rt {

class sphere : public hittable {
private:
    // 记录球心和球半径
    ray center;
    double radius;

    std::shared_ptr<material> mat;
    aabb bbox;

    // 计算球体uv坐标
    static void get_sphere_uv(const vec3& p, double& u, double& v) {
        // p: 球面上点的坐标 表示从球心指向球面上点的单位向量 unit_vector(p - center)
        // atan2 返回值在[-pi, pi] 之间 
        // asin 返回值在[-pi/2, pi/2] 之间
        auto theta = std::acos(-p.y); // polar angle θ 从y轴向下测量。 -p为对其图形学默认的y轴正向向下的坐标系的适配
        auto phi = std::atan2(-p.z, p.x) + pi; // azimuthal angle φ 从x轴正向开始逆时针测量。 此处的-p 实际作用是相机对齐子午线的操作，此处指向太平洋
        u = phi / (2 * pi); // 将 φ 映射到 [0, 1]
        v = theta / pi; // 将 θ 映射到 [0, 1]
    }

public:
    
    // // 记录球心和球半径
    // point3 center;
    // double radius;
    

    // sphere() {}
    // sphere(point3 cen, double r) : center(cen), radius(r) {};

    sphere() {}
    // Stationary Sphere
    sphere(const point3& cen, double r ,shared_ptr<material> _mat) : center(cen,vec3(0,0,0),0.0), radius(std::fmax(0,r)) ,mat(_mat) {
        // 提前计算包围盒，避免每次求交时重复计算
        auto rvec = vec3(radius, radius, radius);
        bbox = aabb(cen - rvec, cen + rvec);
    };
    
    // Moving Sphere
    sphere(const point3& cen1,const point3& cen2, double r ,shared_ptr<material> _mat) : center(cen1, cen2-cen1,0.0), radius(std::fmax(0,r)) ,mat(_mat) {
        // 提前计算包围盒，避免每次求交时重复计算
        auto rvec = vec3(radius, radius, radius);
        aabb box0(cen1 - rvec, cen1 + rvec);
        aabb box1(cen2 - rvec, cen2 + rvec);
        bbox = aabb(box0, box1); // 运动球体包围盒必须包含球体在运动过程中所有位置的包围盒，即两个静止包围盒的并集
    };

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 按照直觉Origin to Center定义 相机指向球心的向量
        // 新增：此时球心由向量计算
        point3 current_center = center.at(r.time()); // 光线时间计算球心
        // rt::vec3 oc = center - r.orig;
        vec3 oc = current_center - r.orig;
        // 由 任意点在球面公式 (P-C)点乘(P-C) 等于 r^2 与 光线方程 P = A + td 的组合展开可得类一元二次方程系数值
        // 即 a= d dot d 
        // auto a = rt::dot(r.dir, r.dir);
        auto a = r.dir.length_squared();
        // // b = 2(v dot d) = 2((A-C) dot d) .又定义 oc = C-A 所以此处需要添加负号
        // // 本质为向量(A-C) 在光线方向的投影长度*2 
        // auto b = -2.0 * rt::dot(oc, r.dir);
        // half_b 优化
        // b = 2*(oc dot r.dir) 使b=-2h 代入原求根等式
        auto h = rt::dot(r.dir, oc);
        // c = (v dot v) - r^2
        // 本质为比较 相机到球心距离和球半径的值大小，可得 c>0 相机在球外、c<0：相机在球里面, c=0 相机在球面
        // auto c = rt::dot(oc, oc) - radius * radius;
        auto c = oc.length_squared() - radius * radius;
        // 求解判别式delta b^2 -4ac ，即大于或者等于0 的情况有解
        auto discriminant = h * h - a * c;
        // return ((b * b - 4 * a * c) >= 0 );

        // if (discriminant < 0) {
        //     return  -1.0;
        // } else {
        //     // 需要最近的交点（较小的t） 即(-b - sqrt(delta)) / 2a  
        //     // return (-b - std::sqrt(discriminant)) / (2.0 * a);
        //     return (h - std::sqrt(discriminant)) / a;
        // }

        if (discriminant < 0) return false;
        auto sqrtd = std::sqrt(discriminant);

        // find the nearest root that lies in the acceptable range.
        // 求交加速，本质为判断光线进入物体包围盒时的对任意平行对面的t时间的进入时间组的最大值即为进入盒子的时间，离开盒子的时间即为离开任意对面时间组的最小值。
        // 即 t enter < t exit 则可知光线同包围盒相交
        // 此处为简单的球体判断 进行t有效范围判断 过滤过近和过远的点
        auto root = ( h - sqrtd) / a;
        // 判断大小根是否在有效范围内
        // if (root < ray_t.min || ray_t.max < root) {
        //     root = ( h + sqrtd ) / a;
        //     if (root < ray_t.min || ray_t.max < root)
        //         return false;
        // }
        if (!ray_t.surrounds(root)) {
            root = ( h + sqrtd ) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        // 即hit_record 类储存相关的信息
        // 注意 此处的rec为由hittable_list 求交规则判断条件传如的 临时temp_rec
        rec.t = root; // 有效交点 即t值
        rec.p = r.at(rec.t); // 以ray方程计算交代坐标
        // vec3 outward_normal = (rec.p - center) / radius;  // 交点处法线单位向量（球心指向交点向量）/球半径 
        vec3 outward_normal = (rec.p - current_center) / radius;
        rec.set_face_normal(r, outward_normal); // 法向量修正

        get_sphere_uv(outward_normal, rec.u, rec.v); // 计算球面UV坐标
        
        rec.mat = mat;
        
        return true;
    }
aabb bounding_box() const override { return bbox; }
// private:
//     // 记录球心和球半径
//     point3 center;
//     double radius;

//     std::shared_ptr<material> mat;
};
}