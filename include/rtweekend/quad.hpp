#pragma once

#include "rtweekend.hpp"
#include "hittable.hpp"
#include "rtweekend/aabb.hpp"
#include "rtweekend/ray.hpp"
#include "rtweekend/vec3.hpp"
#include "rtweekend/hittable_list.hpp"

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

        area = n.length();
        
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

    // 1. 在四边形面上随机挑一个点，返回从 origin 指向那个点的方向
    vec3 random(const point3& origin) const override {
        // 利用平面向量 u 和 v，加上两个 [0,1] 的随机数，就能在平行四边形内随机漫步！
        auto p = Q + (random_double() * u) + (random_double() * v);
        return p - origin; 
    }
    // 2. 如果我们朝着方向 v 射出一根光线，它打中这个四边形的概率密度是多少？
    double pdf_value(const point3& origin, const vec3& direction) const override {
        hit_record rec;
        // 先测试这根光线到底能不能打中这个四边形
        if (!this->hit(ray(origin, direction, 0.0), interval(0.001, infinity), rec))
            return 0;

        auto distance_squared = rec.t * rec.t * direction.length_squared();
        auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

        // 核心数学：将面积上的概率转换成立体角上的概率
        return distance_squared / (cosine * area);
    }

private:
    point3 Q; // 四边形的一个顶点
    vec3 u, v;   // 四边形的两个边向量
    vec3 w; // 用于快速计算交点坐标的辅助向量
    shared_ptr<material> mat;
    aabb bbox; // 四边形的包围盒
    vec3 normal; // 四边形的法线
    double D; // 平面方程中的D参数
    double area; // 四边形的面积

};

// 长方体生成器 (返回一个包含了 6 个 quad 的 hittable_list)
inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat) {
    // 返回值是一个包裹了 6 个面的列表
    auto sides = make_shared<hittable_list>();

    // 确保算出正确的最小/最大坐标点
    auto min = point3(std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z));
    auto max = point3(std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z));

    // 计算盒子的长、宽、高对应的三个基本向量
    auto dx = vec3(max.x - min.x, 0, 0);
    auto dy = vec3(0, max.y - min.y, 0);
    auto dz = vec3(0, 0, max.z - min.z);

    // 拼装 6 个面 (注意向量的朝向，利用叉乘右手定则确保法线全部朝外)
    
    // 正面 (Front)
    sides->add(make_shared<quad>(point3(min.x, min.y, max.z),  dx,  dy, mat));
    // 右面 (Right)
    sides->add(make_shared<quad>(point3(max.x, min.y, max.z), -dz,  dy, mat));
    // 背面 (Back)
    sides->add(make_shared<quad>(point3(max.x, min.y, min.z), -dx,  dy, mat));
    // 左面 (Left)
    sides->add(make_shared<quad>(point3(min.x, min.y, min.z),  dz,  dy, mat));
    // 顶面 (Top)
    sides->add(make_shared<quad>(point3(min.x, max.y, max.z),  dx, -dz, mat));
    // 底面 (Bottom)
    sides->add(make_shared<quad>(point3(min.x, min.y, min.z),  dx,  dz, mat));

    return sides;
}

}