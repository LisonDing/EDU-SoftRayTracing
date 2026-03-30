// 基础轴对齐包围盒 (AABB) 类定义

#pragma once

#include "rtweekend.hpp"
#include "interval.hpp"
#include "vec3.hpp"
#include "ray.hpp"

namespace rt {

class aabb {
public:
    interval x, y, z;

    aabb() {} // 默认空包围盒

    // 构造函数接受三个轴向的区间
    aabb(const interval& ix, const interval& iy, const interval& iz)
      : x(ix), y(iy), z(iz) {}

    // 从两个点构造包围盒，自动判断最小最大坐标
    aabb(const point3& a, const point3& b) {
        // Treat the two points a and b as extrema for the bounding box, so we don't require a
        // particular minimum/maximum coordinate order.

        x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
        y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
        z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);
    }
    
    // 通过两个现有的 AABB 构造一个更大的 AABB（用于合并节点）
    aabb(const aabb& box0, const aabb& box1) {
        x = interval(box0.x, box1.x);
        y = interval(box0.y, box1.y);
        z = interval(box0.z, box1.z);
    }
    
    const interval& axis(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    // 核心算法：判断光线是否击中这个 AABB 包围盒
    // 这使用的是由 Andrew Kensler 优化的 Slab (平板) 求交算法
    bool hit(const ray& r, interval ray_t) const {
        for (int a = 0; a < 3; a++) {
            auto invD = 1.0 / r.direction()[a];
            auto t0 = (axis(a).min - r.origin()[a]) * invD;
            auto t1 = (axis(a).max - r.origin()[a]) * invD;

            if (invD < 0.0f)
                std::swap(t0, t1);

            if (t0 > ray_t.min) ray_t.min = t0;
            if (t1 < ray_t.max) ray_t.max = t1;

            if (ray_t.max <= ray_t.min)
                return false; // 光线错过了包围盒
        }
        return true;
    }

    // 获取最长轴索引，供 BVH 构造时选择分割轴使用  
    int longest_axis() const {
        // Returns the index of the longest axis of the bounding box.

        if (x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        else
            return y.size() > z.size() ? 1 : 2;
    }

    static const aabb empty, universe;
};
const aabb aabb::empty(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe(interval::universe, interval::universe, interval::universe);
}