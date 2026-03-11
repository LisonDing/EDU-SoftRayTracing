#pragma once

#include "rtweekend.hpp"
#include "aabb.hpp"
#include "hittable.hpp"
#include "hittable_list.hpp"

#include <algorithm> // 为了使用 std::sort

namespace rt {

class bvh_node : public hittable {
public:
    // 顶层构造函数：接收整个 hittable_list
    // 委托构造：直接调用核心递归构造函数，传入对象数组和范围
    bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {}

    // 核心递归构造函数
    bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
        // // 1. 随机选择一个轴来排序 (0:X, 1:Y, 2:Z)
        // int axis = random_int(0, 2);
        
        // Build the bounding box of the span of source objects.
        bbox = aabb::empty;
        for (size_t object_index=start; object_index < end; object_index++)
            bbox = aabb(bbox, objects[object_index]->bounding_box());

        int axis = bbox.longest_axis();

        // 2. 选定比较函数
        auto comparator = (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare;

        size_t object_span = end - start;

        // 3. 递归的终止条件与节点分配
        if (object_span == 1) {
            // 只有一个物体：左右子树都指向它（避免空指针判断，稍微浪费点计算但代码简洁）
            left = right = objects[start];
        } else if (object_span == 2) {
            // 只有两个物体：按轴排序后，一个左一个右
            if (comparator(objects[start], objects[start+1])) {
                left = objects[start];
                right = objects[start+1];
            } else {
                left = objects[start+1];
                right = objects[start];
            }
        } else {
            // 多于两个物体：排序，然后从中间劈开，继续递归！
            std::sort(objects.begin() + start, objects.begin() + end, comparator);

            auto mid = start + object_span / 2;
            left = make_shared<bvh_node>(objects, start, mid);
            right = make_shared<bvh_node>(objects, mid, end);
        }
        // 4. 算出当前节点的巨大包围盒（包住左右两个子节点）
        // bbox = aabb(left->bounding_box(), right->bounding_box());
    }

    // 求交函数
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 首先检查光线是否与当前节点的包围盒相交
        if (!bbox.hit(r, ray_t))
            return false;
        // 如果相交，继续检查左右子节点
        bool hit_left = left->hit(r, ray_t, rec);
        // 关键优化：如果左子树碰到了，那右子树只需要在更近的距离内寻找交点
        // 所以把 ray_t.max 替换为 hit_left ? rec.t : ray_t.max
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }
    aabb bounding_box() const override { return bbox; }

private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox; // 当前节点的包围盒

    // --- 排序辅助函数 ---
    static bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index) {
        // 获取两个物体在指定轴上的包围盒区间，并比较它们的最小值
        auto a_axis_interval = a->bounding_box().axis(axis_index);
        auto b_axis_interval = b->bounding_box().axis(axis_index);
        // 比较该轴区间最小值返回升序结果
        return a_axis_interval.min < b_axis_interval.min;
    }

    // 轴特化比较函数，分别针对X、Y、Z轴调用通用比较函数
    static bool box_x_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) { return box_compare(a, b, 0); }
    static bool box_y_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) { return box_compare(a, b, 1); }
    static bool box_z_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) { return box_compare(a, b, 2); }
};

}