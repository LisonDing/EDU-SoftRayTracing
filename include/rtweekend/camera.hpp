// 相机参数

#pragma once

#include "rtweekend.hpp"
#include "rtweekend/interval.hpp"
#include "rtweekend/vec3.hpp"
#include "rtweekend/material.hpp"
#include "rtweekend/pdf.hpp"
// 实现置入cpp
#include <cmath>
#include <stb/stb_image_write.h>

namespace rt {

class camera {
public:
    // 需要默认初始值，确保计算前被正确赋值
    double aspect_ratio = 1.0; // 图像长宽比
    int image_width = 100; // renered image width in pixel count
    int samples_per_pixel = 10;  // 采样次数
    int max_depth = 10; // Maximum number of ray bounces into scene （避免无限递归，控制材质反射 / 折射的计算次数）
    color background; // Scene

    double vfov = 90;  // 垂直视场角（Field of View），决定相机的 “视野宽窄”
    point3 lookfrom = point3(0,0,0);   // Point camera is looking from // 相机原点
    point3 lookat   = point3(0,0,-1);  // Point camera is looking at
    vec3   vup      = vec3(0,1,0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus


    // 封装渲染循环
    void render(const hittable& world , const char* filename , const hittable& lights) {
        initialize();

        int channel_num = 3;
        // 预分配内存，避免 push_back 时的 realloc，性能更优
        std::vector<unsigned char> data;
        // reserve(n)：为 vector预分配 n 个字节的连续内存空间
        data.reserve(image_width * image_height * channel_num); // channel_num = 3 (R,G,B) 通道数

        // // 倒序扫描行 从上到下，目的是将数据按图形存储顺序写入 （push_back 像素数据是从前往后存储的即从左上到右下
        // for (int j = image_height - 1; j >= 0; --j) {
        // 结合新的camera pixel00_loc 的位置进行正序扫描
        for (int j = 0; j < image_height; ++j) {
            //std::clog：C++ 的日志输出流 和std::cout类似，但专门用于输出程序运行日志，不会和普通输出混在一起，且无缓冲区（输出更及时）
            // \r：回车符（不是换行符\n），作用是将控制台的光标回到当前行的开头，而非换行到下一行 —— 这样后续的进度数字会覆盖当前行的旧数字，实现「单行动态刷新进度」的效果；
            // std::flush：强制刷新输出缓冲区，让进度信息立即显示在控制台，避免因缓冲区满而延迟输出
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i) {
                // rt::color pixel_color(
                //     double(i) / (image_width - 1),
                //     double(j) / (image_height - 1),
                //     0.25
                // );
                color pixel_color(0,0,0);

                // 抗锯齿循环
                // 前置自增无临时对象开销
                // for (int s = 0; s < samples_per_pixel; ++s) {
                //     // // 像素中心的随机偏移
                //     // vec3 offset = sample_square();
                //     // // 计算 u,v 参数
                //     // auto u = (double(i) + offset.x) / (image_width - 1);
                //     // auto v = (double(j) + offset.y) / (image_height - 1);

                //     // create ray
                //     // 即viewport任意点p点3D坐标为 基准点+空间基准轴向的位移
                //     ray r = get_ray(i,j); 

                //     // 计算像素颜色
                //     // 累加颜色
                //     pixel_color += ray_color(r, world, max_depth);
                // }

                // 分层采样：先计算每个像素的中心点的颜色，后续再做抗锯齿
                for (int s_j = 0; s_j < sqrt_spp; s_j++) {
                    for (int s_i = 0; s_i < sqrt_spp; s_i++) {
                        ray r = get_ray(i, j, s_i, s_j);
                        pixel_color += ray_color(r, world ,max_depth, lights); // 目前先把lights参数传world，后续再单独传入光源集合
                    }
                }





                //  调试：颜色log
                // if (i == image_width / 2 && j == image_height - 1) {
                //     std::cout << "Check Pixel: " << int(255.99 * pixel_color.x) << " " 
                //                                 << int(255.99 * pixel_color.y) << " " 
                //                                 << int(255.99 * pixel_color.z) << std::endl;
                // }
                

                // samples_per_pixel 暂时传 1，后面做抗锯齿
                rt::write_color_to_buffer(data, pixel_color,samples_per_pixel);
            }
        }

        // stide_bytes 行步幅：每行像素数据的字节数， 目的是告诉 stbi_write_png 每行数据的起始位置 访问对齐内存地址可以提高性能
        stbi_write_png(filename, image_width, image_height, channel_num, data.data(), image_width * channel_num);
        std::clog << "\nDone.\n";
    }

    // point3 origin;
    // point3 lower_left_corner;
    // vec3 horizontal;
    // vec3 vertical;
    


private:
    int image_height;

    double pixel_samples_scale;
    int sqrt_spp; // 每行/列的采样数（samples_per_pixel 的平方根），用于分层采样
    double recip_sqrt_spp; // 1/sqrt_spp 的预计算值，避免在采样循环中重复计算

    // point3 origin;
    // point3 lower_left_corner;
    // vec3 horizontal;
    // vec3 vertical;
    point3 center;         
    point3 pixel00_loc;    
    vec3   pixel_delta_u;  
    vec3   pixel_delta_v;  
    vec3   u, v, w;  // Camera frame basis vectors
    
    // 散焦盘的水平 / 垂直半径向量（景深用），用于生成散焦区域的随机光线原点
    vec3   defocus_disk_u; // Defocus disk horizontal radius
    vec3   defocus_disk_v; // Defocus disk vertical radius

    // 初始化内部变量 用于构建相机同视口的空间关系
    void initialize() {
        // 推导图像高度
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        // 预计算采样相关的值，避免在采样循环中重复计算
        sqrt_spp = int(std::sqrt(samples_per_pixel));
        pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
        recip_sqrt_spp = 1.0 / sqrt_spp;

        // samples_per_pixel = 1.0 / samples_per_pixel;

        center = lookfrom; 

        // Determine viewport dimensions.
        // auto focal_length = 1.0;
        // auto focal_length = (lookfrom - lookat).length();
        
        // 通过定义焦距的方式定义视场
        auto theta = degrees_to_radians(vfov); // 由视场值计算角弧度
        auto h = std::tan(theta/2); // 视场角正切值
        auto viewport_height = 2 * h * focus_dist; // 视口高度 = 2*tan(θ/2)*对焦距离
        auto viewport_width = viewport_height * (double(image_width)/image_height); // // 视口宽度（匹配图像宽高比）

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat); // 相机后向的向量，即从目标点到相机原点
        u = unit_vector(cross(vup, w)); //  // 保持正交叉乘得出 相机右向向量
        v = cross(w, u); //  // 保持正交叉乘得出 相机上向向量

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        // auto viewport_u = vec3(viewport_width, 0, 0);
        // auto viewport_v = vec3(0, -viewport_height, 0);
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        // 此处取反即保证视口像素绘制同步图像储存顺序，等同反向遍历
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge 

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        // 步长计算
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        // 物理成像到视口（此处即等同焦平面）的位移映射
        auto viewport_upper_left =
            center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        // 目的为模拟真实景深
        // 焦盘半径：由散焦角度和对焦距离决定 半径 = 距离 × tan (半角)
        // *散焦角度是 “焦盘左边缘到右边缘的总角度”，半角才是相机中心到焦盘边缘的角度。 弧度转换为计算实际值（C++三角函数必须计算弧度值）
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        // 散焦盘水平/垂直半径
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;

    }

    
    // camera () {
    //     // viewport 参数
    //     auto viewport_height = 2.0;
    //     auto viewport_width = aspect_ratio * viewport_height;
    //     auto focal_length = 1.0; // 焦距 即相机原点到视口平面的距离

    //     // 此处省去auto 避免局部变量赋值
    //     origin = rt::point3(0, 0, 0); // 相机原点
    //     horizontal = rt::vec3(viewport_width, 0, 0); // 视口 水平向量
    //     vertical = rt::vec3(0, viewport_height, 0); // 视口 垂直向量

    //     // 计算 视口 左下角坐标
    //     // 即viewport任意点p点3D坐标为 基准点+空间基准轴向的位移
    //     lower_left_corner = origin - horizontal / 2 - vertical / 2 - rt::vec3(0, 0, focal_length);
    // }

    // 获取射向uv的光线：生成像素对应的采样光线
    ray get_ray(double i , double j, int s_i, int s_j) const {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // 生成像素内的随机偏移，让光线指向像素内的随机位置（而非固定中心），是抗锯齿的核心
        // auto offset = sample_square();

        auto offset = sample_square_stratified(s_i, s_j);

        // 视口采样点的实际位置
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x) * pixel_delta_u)
                          + ((j + offset.y) * pixel_delta_v);

        // auto ray_origin = center;
        // 景深处理，即确认光线原点位置，由焦散盘采样器过滤
        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        // 实际光线方向
        auto ray_direction = pixel_sample - ray_origin;
        // 随机时间生成
        auto ray_time = random_double();

        // 即此时返回的光线数据是“带随机偏移 + 景深” 的光线，由此信息作为像素点的输入信息
        return ray(ray_origin, ray_direction, ray_time);
    }


    // 将像素坐标置中
    vec3 sample_square() const {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        // 生成单位圆盘内的随机点
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }
    
    // 光线绘制接收hittable集合    
    color ray_color(const rt::ray& r, const hittable& world, int depth, const hittable& lights) {
        hit_record rec;
        
        // 递归中止条件
        if (depth <= 0)
            // 弹射次数超限返回黑色
            return color(0,0,0);

        // // 使用t_min = 0.001 避免浮点数 
        // if (world.hit(r,interval (0.001, infinity), rec)) {
        //     // 可视化法线：将区间 [-1,1] 映射到 [0,1]
        //     // 0.5 该常数位表示反射系数
        //     // return 0.5 * (rec.normal + rt::color(1,1,1));

        //     // uniform hemispherical scattering 均匀半球散射
        //     // rt::vec3 direction = rt::random_on_hemishpere(rec.normal);  // 由法线判别的有效随机光

        //     // // lambertian 反射模型。 即使光线更倾向于在法向方向弹射，降低掠射角的分布概率
        //     // // *单位球和法线的夹角 theta 为[0,180] 之间 所以其 点积值域为[-1，1] 当加上法线模长 其结果永远为>=0, 所以天然位于有效半球内
        //     // rt::vec3 direction = rec.normal + rt::random_in_unit_sphere(); 
        //     // // 此处 ray传值为 rec.p即相交点，即光线弹射的物理逻辑，p成为新的光线出发点
        //     // return 0.5 * ray_color(rt::ray(rec.p, direction), world, depth -1);

        //     ray scattered;
        //     color attenuation;
        //     // 调用材质的scatter方法：计算弹射光线（scattered）和颜色衰减（attenuation）
        //     if (!rec.mat->scatter(r, rec, attenuation, scattered))
        //         return  attenuation * ray_color(scattered, world, depth - 1);
        //     return color(0,0,0);
        // }

        // //光线方向归一化
        // vec3 unit_direction = rt::unit_vector(r.direction());
        // // 计算 t 参数 将 y 分量映射到 [0,1]
        // auto sky_t = 0.5 * (unit_direction.y + 1.0);
        // // 线性插值混合白色和蓝色
        // return (1.0 - sky_t) * color(1.0, 1.0, 1.0) + sky_t * color(0.5, 0.7, 1.0);

        // light source 处理：如果光线没有击中任何物体，返回背景颜色
        if (!world.hit(r, interval(0.001, infinity), rec))
            return background;

        ray scattered;
        color attenuation;
        double pdf_val; // 这根光线生成的概率 p(x)，即散射概率密度函数 (PDF) 的值，后续用于平衡重要性采样的权重计算
        color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

        if (!rec.mat->scatter(r, rec, attenuation, scattered, pdf_val))
            return color_from_emission;

        // Mixture PDF 混合采样：将针对光源的 PDF 和余弦加权的漫反射 PDF 混合，既考虑了重要性采样（朝向光源）又保持了漫反射的随机性
        auto p0 = make_shared<cosine_pdf>(rec.normal);
        auto p1 = make_shared<hittable_pdf>(lights, rec.p);
        mixture_pdf mixed_pdf(p0, p1);
        // 用混合 PDF 生成这根次级光线的真实方向
        scattered = ray(rec.p, mixed_pdf.generate(), r.time());
        // 计算这个方向的真实混合概率密度
        pdf_val = mixed_pdf.value(scattered.direction());


        // 获取 f(x) 的几何散射部分
        double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

        // 获取递归回来的光线颜色 L_i
        color sample_color = ray_color(scattered, world, depth - 1, lights);

        // 计算最终颜色：L_o = L_e + f(x) * L_i * p(x) / pdf_val
        // color color_from_scatter = attenuation * ray_color(scattered, world, depth - 1);
        color color_from_scatter = (attenuation * scattering_pdf * sample_color) / pdf_val;

        return color_from_emission + color_from_scatter;
    };

    vec3 sample_square_stratified(int s_i, int s_j) const {
        // Returns the vector to a random point in the square sub-pixel specified by grid
        // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

        auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
        auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

        return vec3(px, py, 0);
    }
// // 将像素坐标置中
// inline vec3 sample_square() {
//     return vec3(random_double() - 0.5, random_double() - 0.5, 0);
// }

};
}