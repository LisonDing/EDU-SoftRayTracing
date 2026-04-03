// 封装求交流程的极大小值intervel

#pragma once
// #include "rtweekend/rtweekend.hpp"
#include <limits>
namespace rt {

class interval {
public:
    double min, max;

    interval(): min(std::numeric_limits<double>::infinity()),max(-std::numeric_limits<double>::infinity()) {} // 空区间初始化
    interval(double _min, double _max) : min(_min), max(_max) {}

    // 区间合并
    interval(const interval& a, const interval& b) {
        // Create the interval tightly enclosing the two input intervals.
        min = a.min <= b.min ? a.min : b.min;
        max = a.max >= b.max ? a.max : b.max;
    }
    
    // 原教程的 size计算区间大小在核心流程基本没用 属于冗余设计

    // 闭区间判断
    bool contains(double x) const {
        return min <= x && x <= max;
    }

    // 开区间判断
    bool surrounds(double x) const {
        return min < x && x < max;
    }

    // 区间裁剪
    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    // 区间扩展
    // 目的是解决令厚度包围盒（Zero-Thickness Bounding Box）导致的漏光/穿透问题
    interval expand(double delta) const {
        auto padding = delta / 2.0;
        return interval(min - padding, max + padding);
    }

    // 获取区间长度 (aabb.hpp 中的 longest_axis 需要用到)
    double size() const {
        return max - min;
    }

    static const interval empty, universe;

};

// 此处的static 定义其归属于intervel类 不会被实例对象复制实体
// cpp 17以上可以使用inline直接在头文件中初始化静态成员
// inline static const interval empty    = interval(+infinity, -infinity);
// inline static const interval universe = interval(-infinity, +infinity);
const interval interval::empty(std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
const interval interval::universe(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());

}