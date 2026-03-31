#pragma once

#include "rtweekend/rtweekend.hpp"
#include "rtweekend/vec3.hpp"

namespace rt {

class perlin {
public:
    perlin() {
        // randfloat = new double[point_count];
        // for (int i = 0; i < point_count; ++i) {
        //     randfloat[i] = random_double();
        // }
        // 将double数组改为vec3数组，生成随机梯度向量
        randfloat = new vec3[point_count];
        for (int i = 0; i < point_count; ++i) {
            randfloat[i] = unit_vector(random(-1, 1)); // 生成范围在[-1, 1]的随机数，作为梯度向量的分量  
        }

        perm_x = perlin_generate_perm();
        perm_y = perlin_generate_perm();
        perm_z = perlin_generate_perm();
    }   

    ~perlin() {
        delete[] randfloat;
        delete[] perm_x;
        delete[] perm_y;
        delete[] perm_z;
    }
    // 增加线性插值平滑过渡的功能
    // 核心函数：输入 3D 坐标，输出一个 0~1 的平滑噪声值
    double noise(const point3& p) const {
        auto u = p.x - std::floor(p.x);
        auto v = p.y - std::floor(p.y);
        auto w = p.z - std::floor(p.z);
        // // 埃尔米特三次插值 (Hermite Cubic)，让网格边缘的过渡更加平滑
        // u = u * u * (3 - 2 * u);
        // v = v * v * (3 - 2 * v);
        // w = w * w * (3 - 2 * w);
    
        auto i = static_cast<int>(std::floor(p.x));
        auto j = static_cast<int>(std::floor(p.y));
        auto k = static_cast<int>(std::floor(p.z));

        // double c[2][2][2];
        vec3 c[2][2][2]; // 存储随机梯度向量的数组

        // 提取当前点周围 8 个网格顶点的随机值
        for (int di = 0; di < 2; di++) {
            for (int dj = 0; dj < 2; dj++) {
                for (int dk = 0; dk < 2; dk++) {
                    c[di][dj][dk] = randfloat[
                        perm_x[(i + di) & 255] ^
                        perm_y[(j + dj) & 255] ^
                        perm_z[(k + dk) & 255]
                    ];
                }
            }
        }
        // 进行三线性插值 (Trilinear Interpolation)
        // return trilinear_interp(c, u, v, w);
        return perlin_interp(c, u, v, w);
    }

    // 通过增加频率和振幅叠加多个噪声层，形成更复杂的分形噪声（Fractal Noise）
    double turb(const point3& p, int depth=7) const {
        auto accum = 0.0;
        auto temp_p = p;
        auto weight = 1.0;

        // 叠加 depth 次（通常是 7 次）
        for (int i = 0; i < depth; i++) {
            accum += weight * noise(temp_p); // 累加当前频率的噪声
            weight *= 0.5;                   // 下一次振幅减半（影响变弱）
            temp_p *= 2;                     // 下一次频率翻倍（细节变密）
        }

        return std::fabs(accum); // 取绝对值，这会在穿过 0 的地方产生锐利的“折痕”
    }

private:
    static const int point_count = 256;
    // double* randfloat;
    vec3* randfloat; // 存储随机梯度向量的数组
    int* perm_x;
    int* perm_y;
    int* perm_z;   

    static int* perlin_generate_perm() {
        auto p = new int[point_count];
        for (int i = 0; i < point_count; ++i) {
            p[i] = i;
        }
        permute(p, point_count);
        return p;
    }

    static void permute(int* p, int n) {
        for (int i = n-1; i > 0; --i) {
            int target = random_int(0, i);
            int temp = p[i];
            p[i] = p[target];
            p[target] = temp;
        }
    }

    // static double trilinear_interp(double c[2][2][2], double u, double v, double w) {
    //     auto accum = 0.0;
    //     for (int i = 0; i < 2; i++)
    //         for (int j = 0; j < 2; j++)
    //             for (int k = 0; k < 2; k++)
    //                 // 根据距离做差值权重，距离越近权重越大，距离越远权重越小，形成平滑过渡
    //                 accum += (i * u + (1 - i) * (1 - u)) *
    //                          (j * v + (1 - j) * (1 - v)) *
    //                          (k * w + (1 - k) * (1 - w)) * c[i][j][k];
    //     return accum;
    // }

    // 点乘柏林差值算法
    static double perlin_interp(vec3 c[2][2][2], double u, double v, double w) {
        // 埃尔米特平滑 (Hermite smoothing) 提前到这里
        auto uu = u * u * (3 - 2 * u);
        auto vv = v * v * (3 - 2 * v);
        auto ww = w * w * (3 - 2 * w);
        auto accum = 0.0;
        
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                for (int k = 0; k < 2; k++) {
                    // weight_v 是从网格顶点指向当前点的偏移向量
                    vec3 weight_v(u - i, v - j, w - k); 

                    // 不再直接加数值，而是加 顶点向量 与 偏移向量 的 点乘 (dot)，计算梯度对当前点的影响
                    accum += (i * u + (1 - i) * (1 - u)) *
                             (j * v + (1 - j) * (1 - v)) *
                             (k * w + (1 - k) * (1 - w)) * dot(c[i][j][k], weight_v); // 点乘计算梯度对当前点的影响
                }
        return accum;
    }
};
}