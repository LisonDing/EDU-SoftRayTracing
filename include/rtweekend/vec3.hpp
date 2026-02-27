#pragma once

#include <cmath>
#include <iostream>
#include "rtweekend/rtweekend.hpp"


namespace rt {  // 命名空间 在main中以rt：： 或者 using namespace 访问

using std::sqrt; //从标准库命名空间 std 中，引入平方根函数 sqrt，之后代码里直接写 sqrt (...) 就等价于 std::sqrt (...)。

struct vec3 {  // computer graphics 中常用的三维向量类 倾向于直接访问 （Plain old data）。使用class 主要是为了封装相关操作 需要添加public 访问修饰符。
    
    // 升级：使用 union 允许通过不同的名字访问同一内存位置
    // 此处为匿名联合体嵌套匿名结构体，目的为内存共享
    union {  // 匿名联合体 联合体内部成员可以成为外部结构体的直接成员
            double e[3];
            struct {  // 匿名结构体 会成为外层联合体的直接成员，进而成为vec3的直接成员
                double x;
                double y;
                double z;
            };
        }; // vec3的对象可以直接访问e[3]、x、y、z，且四者共享同一块内存，修改x等价于修改e[0]，修改e[2]等价于修改z。 即内存地址重合
    // double e[3]; // 静态数组特性

    // --- 构造函数 ---  初始化对象的成员变量
    vec3() : e{0,0,0} {}
    vec3(double e0, double e1, double e2) : x(e0), y(e1), z(e2) {} //带参构造 指定分量具体值

    // // --- 基础访问器 ---
    // double x() const { return e[0]; }
    // double y() const { return e[1]; }
    // double z() const { return e[2]; }
    // 将其并入 union 结构体成员中，方便直接访问

    // --- 操作符重载 (成员函数) ---
    // 返回值是vec3值（非引用）：向量取反会生成一个新的 vec3 对象（原对象不变），不能返回引用（函数内的临时对象vec3(-e0,-e1,-e2)执行完会析构，引用会变成野指针）。
    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); } // 单目运算符
    // 常性重载（同一运算符，写两个版本适配const/ 非const对象），是自定义类型支持下标访问的标准写法，兼顾只读和读写需求
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; } // 返回引用表示 “返回 e [i] 本身”，而非它的拷贝，所以能做左值赋值（比如v[0] = 10.0;），这是下标能修改分量的根本原因

    // 双目复合赋值运算符 全部修改当前对象，且返回vec3&引用支持链式赋值 （比如v1 += v2 *= 2.0;）
    // 无 const 修饰：因为要修改当前对象的 e 数组，破坏了常量性
    vec3& operator+=(const vec3 &v) { 
        x += v.x; // 代码读起来更像数学公式
        y += v.y;
        z += v.z;
        return *this;
    }

    vec3& operator*=(const double t) {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    vec3& operator/=(const double t) {
        return *this *= 1/t; // 复用*=，代码复用，减少冗余
    }

    // --- 几何工具 ---
    // 单向量的模长运算，即向量分量的平方和开根号
    double length() const {
        return sqrt(length_squared());
    }
    // 模长的平方（避免不必要的开根号计算） 
    // 计算向量长度时常用 平方运算单调递增对非负数判断减少计算量
    // 一般在 需要「实际的模长值」时，才调用length()
    double length_squared() const {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }
    
    // 检查向量是否接近零（防止除零错误或计算精度问题） 
    // 常用于：光线方向判断、碰撞求交检测、颜色噪点过滤等场景
    // 浮点运算没有 “严格相等”，只有 “近似相等”，near_zero()就是实现 “向量是否近似为零” 的标准方式
    bool near_zero() const {
        // 1e-8是工程通用经验值，高低精度范围取值：高精度场景（比如光线追踪的焦散、反射计算）：用更小的阈值，如1e-10、1e-12；低精度场景（比如简单的几何渲染、颜色计算）：用更大的阈值，如1e-6、1e-5；
        const double s = 1e-8; 
        // 三个分量的绝对值都小于s，才返回true
        // std::fabs 是取绝对值函数用于浮点类型 区别于 int 类型的 abs()
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }
};

// --- 类型别名 (语义化) ---
// 别名语义化 main调用例如 rt::color
using point3 = vec3;   // 3D 空间点
using color = vec3;    // RGB 颜色

// --- 工具函数 (全局 inline) ---
// 1.解决头文件全局函数的「多重定义链接错误」（核心目的） C++ 的编译链接规则：非 inline 的全局函数，若在头文件中定义，被多个.cpp 文件包含时，会在每个.cpp 中生成一份函数定义，链接时编译器会发现多个相同的函数符号，报「multiple definition」错误
// 2.内联展开的优化提示：inline 关键字提示编译器将函数体直接插入到调用处，减少函数调用开销（尤其是小函数），但是否内联由编译器决定

// 目的：让 C++ 的标准输出流std::cout能直接打印vec3/point3/color对象，无需自己写打印代码，提升开发调试效率
// 输出流对象必须传引用（ostream不支持拷贝，且传引用能保证链式输出）；
inline std::ostream& operator<<(std::ostream &out, const vec3 &v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

// 2.算术运算符 重载为全局函数 补充成员版+=/-=，实现「纯加法 / 减法」即不修改任何原对象的纯运算
inline vec3 operator+(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

// 分量乘法 Hadamard 积 （对应分量相乘） 常用于颜色运算
// 注意区分点积 和 分量乘法，点积返回标量，分量乘法返回向量
inline vec3 operator*(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

// 标量乘法 支持 两种写法：scalar * vec3 以及 vec3 * scalar
inline vec3 operator*(double t, const vec3 &v) {
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3 &v, double t) {
    return t * v;
}

// 标量除法 复用标量乘法
inline vec3 operator/(vec3 v, double t) {
    return (1/t) * v;
}

// 点积：光照计算的核心
inline double dot(const vec3 &u, const vec3 &v) {
    return u.e[0] * v.e[0]
         + u.e[1] * v.e[1]
         + u.e[2] * v.e[2];
}

// 叉积：构建坐标轴的核心
inline vec3 cross(const vec3 &u, const vec3 &v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

// 单位向量：向量归一化
inline vec3 unit_vector(vec3 v) {
    return v / v.length();
}


inline vec3 random() {
    return vec3(random_double(), random_double(), random_double());
}

inline vec3 random(double min, double max) {
    return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
}
// 单位球内随机向量 即随机光线
inline vec3 random_in_unit_sphere() {
    while (true) {
        // 在 [-1,1] 的立方体内随机取点
        // 点p即原点o 到随机点p的向量op
        auto p = random(-1,1);
        // 返回球内点
        // 考虑极小值下溢至零点情况
        if (1e-160 < p.length_squared() <= 1)
            return p;
    }
}
// 点积判别获得有效半球的光线
inline vec3 random_on_hemishpere(const vec3& normal) {
    vec3 on_unit_sphere = unit_vector(random_in_unit_sphere());
    // 点积判断法线所在半球
    if (dot(on_unit_sphere, normal) > 0.0) 
        return on_unit_sphere;
    else // 反之取反处理
        return -on_unit_sphere;
}

// 计算基于法向量的镜面反射向量方向
inline vec3 reflect(const vec3& v, const vec3& n) {
    // 为得到反射方向向量，需要获取入射方向向量在法向量方向的投影向量（即 v dot n），其同入射方向向量的行程构成反射向量
    // *需取反，即指向外表面的行进向量
    return  v - 2*dot(v,n)* n;
}

// 计算refract 折射
// uv: 入射光线单位向量； n： 法线单位向量 ； etai_over_etat: 折射率之比 (η/η')
// 将入射光线分解为「垂直于法线的分量（perp）」和「平行于法线的分量（parallel）」， 同过正弦角度比求得折射光线
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    // 单位入射光线的反方向向量同法线的点积可得余弦值，*余弦得实数可以判断折射是否发生
    // std::fmin(..., 1.0) 将cos theta限制在 1.0 内
    auto cos_theta = std::fmin(dot(-uv,n), 1.0);
    // 入射垂直分量可以表示为：入射向量 - 平行分量（平行于该点法线），以Snell's Law 可知折射的垂直分量为入射垂直分量乘以折射率比
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    // 以单位向量的模长平方（三角函数）可得平行分量模长； 折射发生于内部，平行分量指向内部，即取反获得折射向量平行分量
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return  r_out_perp + r_out_parallel;
}
// 相机焦散盘，已半径参数调节模糊度
inline vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_double(-1,1), random_double(-1,1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

} // namespace rt