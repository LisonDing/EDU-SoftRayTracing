// 纹理类定义
// 本质是以将空间坐标映射到任意数据的函数对象，通常用于为表面提供颜色信息。
// 现代管线中，作为一个宽泛的数据存储结构的概念定位，可以包含任何对渲染管线有用的数据

#pragma once
#include "rtweekend.hpp"
#include <algorithm>

namespace rt {

class texture {
public:
    virtual ~texture() = default;

    // 纯虚函数：根据纹理坐标 (u,v) 和空间位置 p 返回颜色值
    virtual color value(double u, double v, const point3& p) const = 0;
};  

// solid_color 纯色纹理
// 目的是为了提供一个简单的纹理实现，作为其他纹理的基础组件（比如 checkerboard），同时也支持直接使用纯色纹理为物体上色
class solid_color : public texture {
public:
    solid_color(const color& albedo) : albedo(albedo) {}

    // 委托构造函数，即传入r，g，b三个分量时，构造一个color对象并调用核心构造函数
    solid_color(double red, double green, double blue) : solid_color(color(red, green, blue)) {}

    // 子类必须实现纯虚函数 value 来返回纹理颜色值，此处直接返回固定的 albedo 颜色
    color value(double u, double v, const point3& p) const override {
        return albedo;
    }
private:
    color albedo;
};

// checkerboard 3D棋盘格纹理
class checkerboard : public texture {
public:    
    // 接收两种颜色的纹理指针，支持纯色纹理和更复杂的纹理作为棋盘格的两种颜色
    checkerboard(double scale, shared_ptr<texture> even, shared_ptr<texture> odd)
        // 预计算缩放因子，避免在 value函数中重复计算 1/scale
        : inv_scale(1.0 / scale), even(even), odd(odd) {}
    
    //
    checkerboard(double scale, const color& c1, const color& c2)
        : checkerboard(scale, make_shared<solid_color>(c1), make_shared<solid_color>(c2)) {}

    color value(double u, double v, const point3& p) const override {
        // 通过空间坐标 p 的正弦函数的符号来决定返回哪种颜色，形成3D棋盘格效果
        // floor向下取整，得到整数坐标，整数坐标的奇偶性决定颜色
        // 此处使用的是相同的缩放因子，所以空间被划分为均匀的立方体格子，每个格子对应一个颜色
        auto xInteger = int(floor(inv_scale * p.x));
        auto yInteger = int(floor(inv_scale * p.y));
        auto zInteger = int(floor(inv_scale * p.z));

        // 通过坐标整数部分的奇偶性来决定返回哪种颜色，形成3D棋盘格效果
        // 此处的求和模2的操作是基于存在连续的整数坐标和1的周期性，结合其模2结果的反转特性，从而形成的离散交替的颜色分布
        bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;
        
        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }
private:
    double inv_scale; // 预计算的缩放因子，避免在 value函数中重复计算 1/scale
    shared_ptr<texture> even; // 棋盘格的第一种颜色纹理
    shared_ptr<texture> odd;  // 棋盘格的第二种颜色纹理    
        
};
}