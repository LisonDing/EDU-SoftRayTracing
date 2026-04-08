// 这个类的作用是构造一个局部坐标系，给定一个法线向量 n，构造出一个以 n 为 w 轴的坐标系
// 这个坐标系的 u 和 v 轴是任意的，但必须和 w 轴垂直，这样就可以在这个坐标系中进行局部的采样和计算

#pragma once

#include "rtweekend.hpp"

namespace rt {

class onb {
public:
    onb() {}

    inline vec3 operator[](int i) const { return axis[i]; }
    vec3& operator[](int i) { return axis[i]; }

    vec3 u() const { return axis[0]; }
    vec3 v() const { return axis[1]; }
    vec3 w() const { return axis[2]; }

    // 把局部的向量 (a,b,c) 转换到真实的世界坐标系中
    vec3 local(double a, double b, double c) const {
        return a * u() + b * v() + c * w();
    }

    vec3 local(const vec3& a) const {
        return a.x * u() + a.y * v() + a.z * w();
    }

    // 核心算法：已知法线 n (也就是 w 轴)，构造整个坐标系
    void build_from_w(const vec3& n) {
        axis[2] = unit_vector(n);
        // 随便挑一个辅助向量，只要不和法线平行就行
        vec3 a = (std::fabs(axis[2].x) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
        
        // 叉乘造出互相垂直的坐标轴
        axis[1] = unit_vector(cross(axis[2], a));
        axis[0] = cross(axis[2], axis[1]);
    }

public:
    vec3 axis[3];
};

} // namespace rt