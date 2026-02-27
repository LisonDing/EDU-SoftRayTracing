#pragma once
#include "rtweekend/vec3.hpp"
#include <ctime>

namespace rt {

class ray {
public:
    // 光线属于pod (plain old data)类型，倾向于直接访问成员变量
    point3 orig;
    vec3 dir;

    // 时间变量
    double tm;
    // 默认构造函数 适用于需要动态分配ray对象的场景
    ray() {}
    // 带参构造函数 const引用避免不必要的拷贝
    // ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {}

    // 加入光线的时间信息参数
    ray(const point3& origin, const vec3& direction, double time = 0.0) : orig(origin), dir(direction),tm(time) {}

    // 兼容构造函数：委托给核心构造，默认时间为0（非运动模糊场景用）
    ray(const point3& origin, const vec3& direction) : ray(origin, direction, 0) {}

    // 只读访问器
    const point3& origin() const { return orig; }
    const vec3& direction() const { return dir; }
    // 时间获取接口
    double time() const { return tm; }

    // 计算光线在参数 t 位置的点
    point3 at(double t) const {
        return orig + t * dir;
    }
    
};

}