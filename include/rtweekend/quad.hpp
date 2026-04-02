#pragma once

#include "rtweekend.hpp"
#include "hittable.hpp"
#include "rtweekend/aabb.hpp"
#include "rtweekend/vec3.hpp"

namespace rt {

class quad : public hittable {
public:
    quad() {}
    quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> m) 
        : Q(Q), u(u), v(v), mat(m)
    {   
        // 1. 利用叉乘求出平面的法线向量 n
        auto n = cross(u, v);
        normal = unit_vector(n);
        // 2. 平面方程 Ax + By + Cz = D，求出常数 D
        D = dot(normal, Q);
        // 3. 计算 w 向量，用于极其快速地求解交点是否在四边形内部
        w = n / dot(n, n);


        set_bounding_box();
    }

    virtual void set_bounding_box() {
        // 计算四边形顶点
        auto bbox_diagonal1 = aabb(Q, Q + u + v);
        auto bbox_diagonal2 = aabb(Q + u, Q + v);
        bbox = aabb(bbox_diagonal1, bbox_diagonal2);
    }

    aabb bounding_box() const override {
        return bbox;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        auto denom = dot(normal, r.direction());

        // 如果光线与平面平行，则没有交点
        if (std::abs(denom) < 1e-8) 
            return false;

        // 计算交点参数 t 即光线击中平面的时间
        auto t = (D - dot(normal, r.origin())) / denom;
        // 如果 t 不在有效范围内，则没有交点
        if (!ray_t.contains(t))
            return false;

        // 计算交点坐标 P
        auto intersection = r.at(t);
        // 计算从点 Q 指向交点 P 的向量
        vec3 planar_hitpt_vector = intersection - Q;
        // 通过点积计算交点在 u 和 v 方向上的坐标
        auto alpha = dot(w, cross(planar_hitpt_vector, v));
        auto beta = dot(w, cross(u, planar_hitpt_vector));

        // 判断交点是否在四边形内部
        if (!is_interior(alpha, beta, rec))
            return false;

        // 如果击中，填充 hit_record
        rec.t = t;
        rec.p = intersection;
        rec.mat = mat;
        rec.set_face_normal(r, normal);

        return true;

    }

    virtual bool is_interior(double a, double b, hit_record& rec) const {
        interval unit_empty = interval(0, 1);
        
        // 如果 a(即 alpha) 或 b(即 beta) 超出了 [0, 1] 的范围，说明击中了平面，但在四边形框外面
        if (!unit_empty.contains(a) || !unit_empty.contains(b))
            return false;

        // 如果在内部，顺便把 a 和 b 当作纹理的 UV 坐标存下来！
        rec.u = a;
        rec.v = b;
        return true;
    }

private:
    point3 Q; // 四边形的一个顶点
    vec3 u, v;   // 四边形的两个边向量
    vec3 w; // 用于快速计算交点坐标的辅助向量
    shared_ptr<material> mat;
    aabb bbox; // 四边形的包围盒
    vec3 normal; // 四边形的法线
    double D; // 平面方程中的D参数

};
}