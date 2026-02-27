// 场景管理器 Container 本身为hittable类，目的是对封装的对象物体的管理逻辑，并实现对对象的碰撞检测算法（求交规则）

#pragma once

#include "rtweekend/hittable.hpp"
#include "rtweekend/interval.hpp"

#include <memory>
#include <vector>

namespace rt {

// 命名空间作用域引入
// 共享所有权智能指针 自动管理动态分配的内存（new创建的对象），替代手动delete。 引入计数器管理对象
// std::unique_ptr 不支持多对象共享，此处的vector 管理需要进行 push_back 拷贝指针
using std::shared_ptr;
// 配套创建shared_ptr的首选方式
using std::make_shared;

// 公有派生类标准语法： class 派生类名 : 继承方式 基类名 { ... };
class hittable_list : public hittable {
public:
    // 场景对象池
    // 动态数组储存场景所有可命中物体 元素存储为基类智能指针
    std::vector<shared_ptr<hittable>> objects;

    // 封装场景的 增加、删除、初始化逻辑
    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object) { objects.push_back(object); }


    // 求交规则
    // override 显式重写基类hit，编译器检查签名，避免多态失效
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 临时碰撞记录，储存遍历中每个物体的交点信息
        hit_record temp_rec;
        // 判别标记：击中集合任意物体，初始化为false
        bool hit_anything = false;
        // 记录最近交点t值 此处初始化为最远有效距离
        auto closest_so_far = ray_t.max; // 当前最近的撞击点

        // 遍历所有物体
        // const 为遍历过程只读不修改
        for (const auto& object : objects) {
            // 注意：我们把 t_max 设置为 closest_so_far 这意味着：如果你在 10.0m 处撞到了A，那么对于物体B，我们只关心它是否在 10.0m 以内！
            // 本质为擂台算法
            // 通过多态调用「具体几何体（比如 sphere）」的hit函数 即避免对远处的物体做无效求交计算
            // 逻辑为先传参到sphere检查是否存在「t 在 [t_min, closest_so_far] 范围内」的交点 再返回将有效t作为 closest_so_far
            if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t; // 更新最近距离
                rec = temp_rec;              // 更新最终记录
            }
        }

        return hit_anything;
    }
};

} // namespace rt