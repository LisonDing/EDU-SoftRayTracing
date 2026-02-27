#pragma once

#include "rtweekend/vec3.hpp"
// #include <vector>
// #include <algorithm> // for clamp

#include "rtweekend/interval.hpp"

namespace rt {

// clamp实现在intervel中
// // 辅助函数：将 [0,1] 的浮点颜色值 clamping 到 [0.0, 0.999]
// // 目的为避免在转换为 8-bit 整数时溢出到 256
// // 此处取 0.999 而非 1.0，是因为 256 * 1.0 = 256 会超出 unsigned char 的表示范围 [0,255]
// inline double clamp(double x, double min, double max) {
//     if (x < min) return min;
//     if (x > max) return max;
//     return x;
// }

// 核心功能：将浮点颜色转换为 8-bit 整数并写入缓冲区
// 注意：这里我们设计为向 std::vector 写入，或者你也可以设计为返回一个 struct {r,g,b}
inline void write_color_to_buffer(std::vector<unsigned char>& buffer, color pixel_color, int samples_per_pixel) {
    // samples_per_pixel 参数为像素采样数，目的是在多重采样抗锯齿时对颜色进行平均
    // auto自推导类型
    auto r = pixel_color.x;
    auto g = pixel_color.y;
    auto b = pixel_color.z;

    // 平均采样值
    auto scale = 1.0/ samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;

    // gamma矫正
    r = sqrt(r);
    g = sqrt(g);
    b = sqrt(b);
    
    static const interval intensity(0.000, 0.999);

    // 将 [0,1] 映射到 [0,255]
    // 此处撤职为写入缓冲区而非返回颜色结构体目的是性能优化：避免频繁的内存分配和拷贝
    buffer.push_back(static_cast<unsigned char>(256 * intensity.clamp(r)));
    buffer.push_back(static_cast<unsigned char>(256 * intensity.clamp(g)));
    buffer.push_back(static_cast<unsigned char>(256 * intensity.clamp(b)));

    // buffer.push_back(int(255.999 * r));
    // buffer.push_back(int(255.999 * g));
    // buffer.push_back(int(255.999 * b));
}

} // namespace rt