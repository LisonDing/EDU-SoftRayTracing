// 通用工具 Utilities

#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>
#include <random>


// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions
// 角度转弧度
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

// random num 
// 返回一个 [0, 1) 的实数
inline double random_double() {
    // 分布器：连续均匀分布， 只定义分布规则
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    // 随机数生成器（引擎），是伪随机数的 “源头”，基于梅森旋转算法实现
    static std::mt19937 generator;
    // 即返回设置区间的随机数
    return distribution(generator);
}

// 返回一个 [min, max) 的实数
// 重载线性映射生成 [min, max) 区间的均匀随机数
inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

// Common Headers
#include "color.hpp"
#include "vec3.hpp"
#include "ray.hpp"
#include "interval.hpp"
#include "hittable.hpp"
