// 封装实例对象的变换操作，包含平移、旋转、缩放等

#pragma once

#include "rtweekend.hpp"
#include "hittable.hpp"
#include "aabb.hpp"

namespace rt {

// =================================================================
// 平移变换 (Translate)
// =================================================================
class translate : public hittable {
public:
    translate(shared_ptr<hittable> p, const vec3& displacement)
      : object(p), offset(displacement) 
    {
        // 包围盒也需要跟着平移
        auto orig_bbox = object->bounding_box();
        bbox = aabb(
            interval(orig_bbox.x.min + offset.x, orig_bbox.x.max + offset.x),
            interval(orig_bbox.y.min + offset.y, orig_bbox.y.max + offset.y),
            interval(orig_bbox.z.min + offset.z, orig_bbox.z.max + offset.z)
        );
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 1. 将光线反向平移 (移入物体的局部空间)
        ray offset_r(r.origin() - offset, r.direction(), r.time());

        // 2. 在局部空间测试是否击中
        if (!object->hit(offset_r, ray_t, rec))
            return false;

        // 3. 如果击中了，把交点正向平移回来 (移回世界空间)
        rec.p += offset;

        return true;
    }

    aabb bounding_box() const override { return bbox; }

private:
    shared_ptr<hittable> object;
    vec3 offset;
    aabb bbox;
};

// =================================================================
// 绕 Y 轴旋转变换 (Rotate Y)
// =================================================================
class rotate_y : public hittable {
public:
    rotate_y(shared_ptr<hittable> p, double angle) : object(p) {
        auto radians = degrees_to_radians(angle);
        sin_theta = std::sin(radians);
        cos_theta = std::cos(radians);
        bbox = object->bounding_box();

        // 旋转会改变包围盒的大小！我们需要遍历原包围盒的 8 个顶点，
        // 旋转它们，然后重新计算出一个更大的能包裹住旋转后物体的全新包围盒。
        point3 min( infinity,  infinity,  infinity);
        point3 max(-infinity, -infinity, -infinity);

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
                    auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
                    auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

                    auto newx =  cos_theta * x + sin_theta * z;
                    auto newz = -sin_theta * x + cos_theta * z;

                    vec3 tester(newx, y, newz);
                    for (int c = 0; c < 3; c++) {
                        min[c] = std::fmin(min[c], tester[c]);
                        max[c] = std::fmax(max[c], tester[c]);
                    }
                }
            }
        }
        bbox = aabb(min, max);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 1. 把光线的起点和方向反向旋转 (移入局部空间)
        auto origin = r.origin();
        auto direction = r.direction();

        origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
        origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

        direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
        direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

        ray rotated_r(origin, direction, r.time());

        // 2. 判断是否击中
        if (!object->hit(rotated_r, ray_t, rec))
            return false;

        // 3. 把交点和法线正向旋转回来 (移回世界空间)
        auto p = rec.p;
        p[0] =  cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

        auto normal = rec.normal;
        normal[0] =  cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

        rec.p = p;
        rec.normal = normal;

        return true;
    }

    aabb bounding_box() const override { return bbox; }

private:
    shared_ptr<hittable> object;
    double sin_theta;
    double cos_theta;
    aabb bbox;
};

} // namespace rt