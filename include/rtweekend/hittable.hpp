// 求交规范： 即定义一次撞击时必要信息 撞击点：p； 法线朝向定义：nomal ； 同相机距离： t；  撞击方向： face

#pragma once

#include "rtweekend/ray.hpp"
#include "rtweekend/aabb.hpp"
#include "rtweekend/interval.hpp"
#include "rtweekend/vec3.hpp"
#include <memory>

namespace rt {
// 前置声明
class material;
// 定义碰撞物体时的信息：t：碰撞点在光线的进行时间、 N：碰撞点法线 、 P：碰撞点坐标、Face：光线从物体里还是物体外射出（透明材质
struct hit_record {
    point3 p;
    vec3 normal;
    double t;
    bool front_face; // true 从外部 反之 内部

    // 指向类的智能指针，编译器不需要知道类的详细信息，从而解决循环引用的问题
    std::shared_ptr<material> mat;

    // 辅助强制转换法线指向为光线来的一侧
    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        // 光线方向点乘球体外向法线 如果光线从外射入则同法线方向相反，可判别为<0，即true
        front_face = dot(r.dir, outward_normal) < 0;
        // 以bool值判断光线方向 再取反
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    // 虚析构函数 hittable会被其他子类继承 目的防止内存泄露，即在实际内存开辟时会查询父指针的实际指向对象
    // = default 和 {} 等同 为编译器的显式指令 机器优化的空函数体，一般为团队开发中避免开发者是否忘记写析构逻辑的歧义
    virtual ~hittable() = default;
    
    // hit_record& rec 目的为储存交点信息，即：t值、坐标、法向量，非const&因为要修改
    // =0是 C++ 中纯虚函数（Pure Virtual Function）的专属标志，不是给函数赋值，而是给编译器的指令。 定义借口规范同时禁止直接实例化，强制约束派生类重写基类所有纯虚函数
    // const 位于函数参数列表后即定义常量成员函数（Const Member Function），强制只读类型，即此处hit设计逻辑为纯查询操作。 const修饰的是hit 而非其参数
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const =0;

    // 所有物体必须能返回自己的 AABB
    virtual aabb bounding_box() const = 0;
};


}