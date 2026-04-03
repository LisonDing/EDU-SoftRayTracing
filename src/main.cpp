
#include <iostream>
#include <memory>
#include <vector>

// 保持 STB 实现定义在 cpp 文件中
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <rtweekend/rtweekend.hpp>

#include <rtweekend/hittable_list.hpp>

#include <rtweekend/sphere.hpp>

#include <rtweekend/BVH.hpp>

#include <rtweekend/camera.hpp>

#include <rtweekend/material.hpp>

#include <rtweekend/texture.hpp>

#include <rtweekend/quad.hpp>

#include <rtweekend/transform.hpp>




// rt::color ray_color(const rt::ray& r) {
//     // sphere
//     auto t = hit_sphere(rt::point3(0,0,-1), 0.5, r);
//     // std::cerr << "HIT! t=" << t << "\n";
//     if (t > 0.0) {
//         // 计算撞击点 P = A + td
//         // 此处表示p满足 p-c 即从球心指向球面上的点（也就是半径向量） 即求法线
//         rt::vec3 N = rt::unit_vector(r.at(t) - rt::vec3(0,0,-1));

//         // 颜色分量从[-1, 1] 映射到 [0, 1]
//         return 0.5 * rt::color(N.x + 1, N.y + 1, N.z + 1);
//     }
//     // 光线方向归一化
//     rt::vec3 unit_direction = rt::unit_vector(r.direction());
//     // 计算 t 参数 将 y 分量映射到 [0,1]
//     auto sky_t = 0.5 * (unit_direction.y + 1.0);
//     // 线性插值混合白色和蓝色
//     return (1.0 - sky_t) * rt::color(1.0, 1.0, 1.0) + sky_t * rt::color(0.5, 0.7, 1.0);
// }


// // 光线绘制接收hittable集合    
// rt::color ray_color(const rt::ray& r, const rt::hittable& world, int depth) {
//     rt::hit_record rec;
    
//     // 递归中止条件
//     if (depth <= 0)
//         // 弹射次数超限返回黑色
//         return rt::color(0,0,0);

//     // 使用t_min = 0.001 避免浮点数 
//     if (world.hit(r,rt::interval (0.001, infinity), rec)) {
//         // 可视化法线：将区间 [-1,1] 映射到 [0,1]
//         // 0.5 该常数位表示反射系数
//         // return 0.5 * (rec.normal + rt::color(1,1,1));

//         // uniform hemispherical scattering 均匀半球散射
//         // rt::vec3 direction = rt::random_on_hemishpere(rec.normal);  // 由法线判别的有效随机光

//         // // lambertian 反射模型。 即使光线更倾向于在法向方向弹射，降低掠射角的分布概率
//         // // *单位球和法线的夹角 theta 为[0,180] 之间 所以其 点积值域为[-1，1] 当加上法线模长 其结果永远为>=0, 所以天然位于有效半球内
//         // rt::vec3 direction = rec.normal + rt::random_in_unit_sphere(); 
//         // // 此处 ray传值为 rec.p即相交点，即光线弹射的物理逻辑，p成为新的光线出发点
//         // return 0.5 * ray_color(rt::ray(rec.p, direction), world, depth -1);

//         rt::ray scattered;
//         rt::color attenuation;
//         if (rec.mat->scatter(r, rec, attenuation, scattered))
//             return  attenuation * ray_color(scattered, world, depth - 1);
//         return rt::color(0,0,0);
//     }

//     //光线方向归一化
//     rt::vec3 unit_direction = rt::unit_vector(r.direction());
//     // 计算 t 参数 将 y 分量映射到 [0,1]
//     auto sky_t = 0.5 * (unit_direction.y + 1.0);
//     // 线性插值混合白色和蓝色
//     return (1.0 - sky_t) * rt::color(1.0, 1.0, 1.0) + sky_t * rt::color(0.5, 0.7, 1.0);
// }

// 简单的switch场景选择
rt::hittable_list bouncing_spheres() {

    // world
    rt::hittable_list world;
    // // 中心小球
    // world.add(std::make_shared<rt::sphere>(rt::point3(0,0,-1), 0.5));
    // // 小球2
    // world.add(std::make_shared<rt::sphere>(rt::point3(-0.5,0,-1.5), 0.5));
    // // flood
    // world.add(std::make_shared<rt::sphere>(rt::point3(0,-100.5,-1),100));

    // 场景1
    // auto material_ground = make_shared<rt::lambertian>(rt::color(0.8, 0.8, 0.0));
    // auto material_center = make_shared<rt::lambertian>(rt::color(0.1, 0.2, 0.5));
    // auto material_left   = make_shared<rt::dielectric>(1.50);
    // auto material_bubble = make_shared<rt::dielectric>(1.00 / 1.50);
    // auto material_right  = make_shared<rt::metal>(rt::color(0.8, 0.6, 0.2),1.0);

    // world.add(make_shared<rt::sphere>(rt::point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    // world.add(make_shared<rt::sphere>(rt::point3( 0.0,    0.0, -1.2),   0.5, material_center));
    // world.add(make_shared<rt::sphere>(rt::point3(-1.0,    0.0, -1.0),   0.5, material_left));
    // world.add(make_shared<rt::sphere>(rt::point3(-1.0,    0.0, -1.0),   0.4, material_bubble));
    // world.add(make_shared<rt::sphere>(rt::point3( 1.0,    0.0, -1.0),   0.5, material_right));

    // 场景2
    // auto R = std::cos(pi/4);

    // auto material_left  = make_shared<rt::lambertian>(rt::color(0,0,1));
    // auto material_right = make_shared<rt::lambertian>(rt::color(1,0,0));

    // world.add(make_shared<rt::sphere>(rt::point3(-R, 0, -1), R, material_left));
    // world.add(make_shared<rt::sphere>(rt::point3( R, 0, -1), R, material_right));

    // 场景3 cover
    // 地面球体
    // auto ground_material = make_shared<rt::lambertian>(rt::color(0.5, 0.5, 0.5));
    // world.add(make_shared<rt::sphere>(rt::point3(0,-1000,0), 1000, ground_material));

    // 场景4 棋盘格
    auto checker = make_shared<rt::checkerboard>(0.32, rt::color(0.2, 0.3, 0.1), rt::color(0.9, 0.9, 0.9));
    world.add(make_shared<rt::sphere>(rt::point3(0,-1000,0), 1000, make_shared<rt::lambertian>(checker)));

    // 双层循环矩阵 21x21
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            // 随机数决定小球材质类型
            auto choose_mat = random_double();
            // 生成球心坐标：网格基础位置 + 随机偏移
            rt::point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            // 碰撞规避：避免小球与右侧大金属球重叠
            if ((center - rt::point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<rt::material> sphere_material;

                // 材质分配概率
                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = rt::random() * rt::random();
                    sphere_material = make_shared<rt::lambertian>(albedo);

                    auto center2 = center + rt::vec3(0,random_double(0,.5),0);
                    world.add(make_shared<rt::sphere>(center, center2, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = rt::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<rt::metal>(albedo, fuzz);
                    world.add(make_shared<rt::sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<rt::dielectric>(1.5);
                    world.add(make_shared<rt::sphere>(center, 0.2, sphere_material));
                }
                //  world.add(make_shared<rt::sphere>(center, center2, 0.2, sphere_material));
            }
        }
    }

    auto material1 = make_shared<rt::dielectric>(1.5);
    world.add(make_shared<rt::sphere>(rt::point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<rt::lambertian>(rt::color(0.4, 0.2, 0.1));
    world.add(make_shared<rt::sphere>(rt::point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<rt::metal>(rt::color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<rt::sphere>(rt::point3(4, 1, 0), 1.0, material3));

    return world;
}


rt::hittable_list checkered_spheres() {
    // world
    rt::hittable_list world;

    auto checker = make_shared<rt::checkerboard>(0.32, rt::color(0.2, 0.3, 0.1), rt::color(0.9, 0.9, 0.9));
    world.add(make_shared<rt::sphere>(rt::point3(0,-10, 0), 10, make_shared<rt::lambertian>(checker)));
    world.add(make_shared<rt::sphere>(rt::point3(0,10, 0), 10, make_shared<rt::lambertian>(checker)));

    // rt::camera cam;

    return world;

 
}

rt::hittable_list earth() {
    rt::hittable_list world;

    auto earth_texture = make_shared<rt::image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<rt::lambertian>(earth_texture);
    world.add(make_shared<rt::sphere>(rt::point3(0,0,0), 2, earth_surface));

    return world;
}

rt::hittable_list perlin_spheres() {
    rt::hittable_list world;

    auto pertext = make_shared<rt::noise_texture>(4.0);
    auto noise_mat = make_shared<rt::lambertian>(pertext);
    world.add(make_shared<rt::sphere>(rt::point3(0,-1000,0), 1000, noise_mat));
    world.add(make_shared<rt::sphere>(rt::point3(0,2,0), 2, noise_mat));
    return world;
}
rt::hittable_list quads() {
    rt::hittable_list world;

    // 各种不同颜色的纯色材质
    auto left_red     = make_shared<rt::lambertian>(rt::color(1.0, 0.2, 0.2));
    auto back_green   = make_shared<rt::lambertian>(rt::color(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<rt::lambertian>(rt::color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<rt::lambertian>(rt::color(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<rt::lambertian>(rt::color(0.2, 0.8, 0.8));

    // 创建五个分布在空间不同位置和朝向的四边形
    world.add(make_shared<rt::quad>(rt::point3(-3,-2, 5), rt::vec3(0, 0,-4), rt::vec3(0, 4, 0), left_red));
    world.add(make_shared<rt::quad>(rt::point3(-2,-2, 0), rt::vec3(4, 0, 0), rt::vec3(0, 4, 0), back_green));
    world.add(make_shared<rt::quad>(rt::point3( 3,-2, 1), rt::vec3(0, 0, 4), rt::vec3(0, 4, 0), right_blue));
    world.add(make_shared<rt::quad>(rt::point3(-2, 3, 1), rt::vec3(4, 0, 0), rt::vec3(0, 0, 4), upper_orange));
    world.add(make_shared<rt::quad>(rt::point3(-2,-3, 5), rt::vec3(4, 0, 0), rt::vec3(0, 0,-4), lower_teal));

    // // Debug: print bounding boxes of each object in the scene
    // for (size_t i = 0; i < world.objects.size(); ++i) {
    //     auto b = world.objects[i]->bounding_box();
    //     std::clog << "Object " << i << " bbox x:[" << b.axis(0).min << "," << b.axis(0).max << "] "
    //               << "y:[" << b.axis(1).min << "," << b.axis(1).max << "] "
    //               << "z:[" << b.axis(2).min << "," << b.axis(2).max << "]\n";
    // }

    return world;
}

rt::hittable_list simple_light() {
    rt::hittable_list world;

    auto pertext = make_shared<rt::noise_texture>(4.0);
    world.add(make_shared<rt::sphere>(rt::point3(0,-1000,0), 1000, make_shared<rt::lambertian>(pertext)));
    world.add(make_shared<rt::sphere>(rt::point3(0,2,0), 2, make_shared<rt::lambertian>(pertext)));

    auto difflight = make_shared<rt::diffuse_light>(rt::color(4,4,4));
    world.add(make_shared<rt::quad>(rt::point3(3,1,-2), rt::vec3(2, 0,0), rt::vec3(0,2,0), difflight));
    world.add(make_shared<rt::sphere>(rt::point3(0,7,0), 2, difflight));

    return world;
}

rt::hittable_list cornell_box() {
    rt::hittable_list world;

    auto red   = make_shared<rt::lambertian>(rt::color(0.65, 0.05, 0.05));
    auto white = make_shared<rt::lambertian>(rt::color(0.73, 0.73, 0.73));
    auto green = make_shared<rt::lambertian>(rt::color(0.12, 0.45, 0.15));
    auto light = make_shared<rt::diffuse_light>(rt::color(15, 15, 15));

    world.add(make_shared<rt::quad>(rt::point3(555,0,0), rt::vec3(0,555,0), rt::vec3(0,0,555), green)); // left
    world.add(make_shared<rt::quad>(rt::point3(0,0,0), rt::vec3(0,555,0), rt::vec3(0,0,555), red)); // right
    world.add(make_shared<rt::quad>(rt::point3(343, 554, 332), rt::vec3(-130,0,0), rt::vec3(0,0,-105), light)); // light
    world.add(make_shared<rt::quad>(rt::point3(0,0,0), rt::vec3(555,0,0), rt::vec3(0,0,555), white)); // ceiling
    world.add(make_shared<rt::quad>(rt::point3(555,555,555), rt::vec3(-555,0,0), rt::vec3(0,0,-555), white)); // floor
    world.add(make_shared<rt::quad>(rt::point3(0,0,555), rt::vec3(555,0,0), rt::vec3(0,555,0), white)); // back

    // 实例化：矮盒子 (先建模型 -> 旋转 -18度 -> 平移)
    shared_ptr<rt::hittable> box1 = rt::box(rt::point3(0,0,0), rt::point3(165,165,165), white);
    box1 = make_shared<rt::rotate_y>(box1, -18);
    box1 = make_shared<rt::translate>(box1, rt::vec3(130,0,65));
    world.add(box1);
    
    // 实例化：高盒子 (先建模型 -> 旋转 15度 -> 平移)
    shared_ptr<rt::hittable> box2 = rt::box(rt::point3(0,0,0), rt::point3(165,330,165), white);
    box2 = make_shared<rt::rotate_y>(box2, 15);
    box2 = make_shared<rt::translate>(box2, rt::vec3(265,0,295));
    world.add(box2);

    return world;
}


int main () {
    // 1. 初始化空的世界和相机
    rt::hittable_list world;
    rt::camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    // cam.samples_per_pixel = 100;
    // cam.image_width       = 1200;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    // default scene background color
    cam.background        = rt::color(0,0,0);
    
    // Specifying the color of the background
    // cam.background        = rt::color(0.70, 0.80, 1.00);

    // cam.vfov = 20;
    // // cam.lookfrom = rt::point3(-2,2,1);
    // // cam.lookat   = rt::point3(0,0,-1);
    // // cam.vup      = rt::vec3(0,1,0);
    // cam.lookfrom = rt::point3(13,2,3);
    // cam.lookat   = rt::point3(0,0,0);
    cam.vup      = rt::vec3(0,1,0);
    
    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;
    // 2. 场景选择
    int switch_sence = 7;
    switch (switch_sence) {
        case 1:
            world = bouncing_spheres();
            cam.vfov = 20;
            // cam.lookfrom = rt::point3(-2,2,1);
            // cam.lookat   = rt::point3(0,0,-1);
            // cam.vup      = rt::vec3(0,1,0);
            cam.lookfrom = rt::point3(13,2,3);
            cam.lookat   = rt::point3(0,0,0);
            // cam.vup      = rt::vec3(0,1,0);
            break;  
        case 2:
            world = checkered_spheres();
            cam.lookfrom = rt::point3(13, 2, 3);
            cam.lookat   = rt::point3(0, 0, 0);
            cam.vfov     = 20.0;
            break;
        case 3:
            world = earth();
            cam.lookfrom = rt::point3(0, 0, 12);
            cam.lookat   = rt::point3(0, 0, 0);
            cam.vfov     = 20.0;
            break;
        case 4:
            world = perlin_spheres();
            cam.lookfrom = rt::point3(13,2,3);
            cam.lookat   = rt::point3(0, 0, 0);
            cam.vfov     = 20.0;
            break;
        case 5:
            world = quads();
            cam.aspect_ratio      = 1.0;     // 改成正方形画幅
            cam.image_width       = 400;
            cam.samples_per_pixel = 100;
            cam.max_depth         = 50;
            
            cam.lookfrom = rt::point3(0, 0, 9); // 相机正对着 Z 轴看过去
            cam.lookat   = rt::point3(0, 0, 0);
            cam.vfov     = 80.0;             // 广角镜头
            cam.defocus_angle = 0; // 关闭景深效果
            break;
        case 6:
            world = simple_light();
            cam.lookfrom = rt::point3(26,3,6);
            cam.lookat   = rt::point3(0,2,0);
            cam.vfov     = 20;

            cam.defocus_angle = 0;
            break;
        case 7:
            world = cornell_box();
            // 经典画幅：正方形
            cam.aspect_ratio      = 1.0;
            cam.image_width       = 600;     // 分辨率可以稍微开高点
            cam.samples_per_pixel = 200;     // 室内光追噪点较多，建议 200 以上采样
            cam.max_depth         = 50;

            // 相机放在 Z轴 -800 的位置，往盒子中心 (278,278,0) 看
            cam.lookfrom = rt::point3(278, 278, -800);
            cam.lookat   = rt::point3(278, 278, 0);
            cam.vup      = rt::vec3(0, 1, 0);
            cam.vfov     = 40.0;
            break;
    }
    // 将线性list置入BVH节点树 以加速求交
    world = rt::hittable_list(make_shared<rt::bvh_node>(world));

    // cam.aspect_ratio      = 16.0 / 9.0;
    // cam.image_width       = 400;
    // // cam.samples_per_pixel = 100;
    // // cam.image_width       = 1200;
    // cam.samples_per_pixel = 100;
    // cam.max_depth         = 50;

    // // cam.vfov = 20;
    // // // cam.lookfrom = rt::point3(-2,2,1);
    // // // cam.lookat   = rt::point3(0,0,-1);
    // // // cam.vup      = rt::vec3(0,1,0);
    // // cam.lookfrom = rt::point3(13,2,3);
    // // cam.lookat   = rt::point3(0,0,0);
    // cam.vup      = rt::vec3(0,1,0);

    // cam.defocus_angle = 10.0;
    // cam.focus_dist    = 3.4;
    // cam.defocus_angle = 0.6;
    // cam.focus_dist    = 10.0;

    cam.render(world,"NRT_image_Instance_26.png");
    // // rt::camera cam(aspect_ratio);
    // // // 采样次数
    // // const int samples_per_pixel = 50;
    
    // // 预分配内存，避免 push_back 时的 realloc，性能更优
    // std::vector<unsigned char> data;
    // // reserve(n)：为 vector预分配 n 个字节的连续内存空间
    // data.reserve(image_width * image_height * channel_num); // channel_num = 3 (R,G,B) 通道数

    // // 倒序扫描行 从上到下，目的是将数据按图形存储顺序写入 （push_back 像素数据是从前往后存储的即从左上到右下
    // for (int j = image_height - 1; j >= 0; --j) {
    //     //std::clog：C++ 的日志输出流 和std::cout类似，但专门用于输出程序运行日志，不会和普通输出混在一起，且无缓冲区（输出更及时）
    //     // \r：回车符（不是换行符\n），作用是将控制台的光标回到当前行的开头，而非换行到下一行 —— 这样后续的进度数字会覆盖当前行的旧数字，实现「单行动态刷新进度」的效果；
    //     // std::flush：强制刷新输出缓冲区，让进度信息立即显示在控制台，避免因缓冲区满而延迟输出
    //     std::clog << "\rScanlines remaining: " << j << ' ' << std::flush;
    //     for (int i = 0; i < image_width; ++i) {
    //         // rt::color pixel_color(
    //         //     double(i) / (image_width - 1),
    //         //     double(j) / (image_height - 1),
    //         //     0.25
    //         // );
    //         rt::color pixel_color(0,0,0);

    //         // 抗锯齿循环
    //         // 前置自增无临时对象开销
    //         for (int s = 0; s < cam.samples_per_pixel; ++s) {
    //             // 像素中心的随机偏移
    //             rt::vec3 offset = cam.sample_square();
    //             // 计算 u,v 参数
    //             auto u = (double(i) + offset.x) / (image_width - 1);
    //             auto v = (double(j) + offset.y) / (image_height - 1);

    //             // create ray
    //             // 即viewport任意点p点3D坐标为 基准点+空间基准轴向的位移
    //             rt::ray r = cam.get_ray(u,v);

    //             // 计算像素颜色
    //             // 累加颜色
    //             pixel_color += ray_color(r, world, max_depth);
    //         }
    //         //  调试：颜色log
    //         // if (i == image_width / 2 && j == image_height - 1) {
    //         //     std::cout << "Check Pixel: " << int(255.99 * pixel_color.x) << " " 
    //         //                                 << int(255.99 * pixel_color.y) << " " 
    //         //                                 << int(255.99 * pixel_color.z) << std::endl;
    //         // }
            

    //         // samples_per_pixel 暂时传 1，后面做抗锯齿
    //         rt::write_color_to_buffer(data, pixel_color,cam.samples_per_pixel);
    //     }
    // }
    
    // // stide_bytes 行步幅：每行像素数据的字节数， 目的是告诉 stbi_write_png 每行数据的起始位置 访问对齐内存地址可以提高性能
    // stbi_write_png("image_dielectric_8.png", image_width, image_height, channel_num, data.data(), image_width * channel_num);
    // std::clog << "\nDone.\n";

}
