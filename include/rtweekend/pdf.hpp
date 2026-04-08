#pragma once

#include "rtweekend.hpp"
#include "onb.hpp" // 需要引入正交基 (稍后我们要创建它)

namespace rt {

class pdf {
public:
    virtual ~pdf() = default;

    virtual double value(const vec3& direction) const = 0;
    virtual vec3 generate() const = 0;
};

// 1. 针对物体的 PDF (朝着光源发射的 PDF)
class hittable_pdf : public pdf {
public:
    hittable_pdf(const hittable& _objects, const point3& _origin)
      : objects(_objects), origin(_origin) {}

    double value(const vec3& direction) const override {
        return objects.pdf_value(origin, direction);
    }

    vec3 generate() const override {
        return objects.random(origin);
    }

private:
    const hittable& objects;
    point3 origin;
};

// 2. 余弦加权 PDF (用于漫反射的采样)
class cosine_pdf : public pdf {
public:
    // 需要传入法线，因为余弦加权是围绕法线展开的
    cosine_pdf(const vec3& w) { uvw.build_from_w(w); }

    double value(const vec3& direction) const override {
        auto cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cosine_theta / pi);
    }

    vec3 generate() const override {
        return uvw.local(random_cosine_direction());
    }

private:
    onb uvw; // Orthonormal Basis (正交基)
};

// 3. 混合 PDF (同时考虑物体和余弦加权)
class mixture_pdf : public pdf {
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }

    // 概率密度值 = 0.5 * PDF_1 + 0.5 * PDF_2
    double value(const vec3& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    // 生成光线：抛硬币，50% 用 PDF_1 生成，50% 用 PDF_2 生成
    vec3 generate() const override {
        if (random_double() < 0.5)
            return p[0]->generate();
        else
            return p[1]->generate();
    }

private:
    shared_ptr<pdf> p[2];
};

} // namespace rt