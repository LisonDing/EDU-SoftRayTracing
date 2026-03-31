// 图片的加载、内存释放和像素读取
#pragma once

// 屏蔽一些 stb 库在某些编译器下的警告
#ifdef _MSC_VER
    #pragma warning (push, 0)
#endif

#include "../../external/stb/stb_image.h"
#include <cstdlib>
#include <iostream>

namespace rt {

class rtw_image {
public:
    rtw_image() : data(nullptr), image_width(0), image_height(0) {}

    rtw_image(const char* image_filename) {
        // 尝试从不同的路径加载图片 (防止因为执行目录不同导致找不到图片)
        auto filename = std::string(image_filename);
        auto imagedir = std::string("assets/images/");

        if (load(filename)) return;
        if (load(imagedir + filename)) return;
        if (load("../" + imagedir + filename)) return;
        if (load("../../" + imagedir + filename)) return;

        std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
    }

    ~rtw_image() {
        stbi_image_free(data);
    }

    bool load(const std::string filename) {
        // 强制加载为 RGB (3通道)
        auto n = bytes_per_pixel;
        data = stbi_load(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
        return data != nullptr;
    }

    int width()  const { return (data == nullptr) ? 0 : image_width; }
    int height() const { return (data == nullptr) ? 0 : image_height; }

    const unsigned char* pixel_data(int x, int y) const {
        // 返回指向对应像素的指针
        static unsigned char magenta[] = { 255, 0, 255 }; // 如果没加载成功，返回显眼的洋红色作为警告
        if (data == nullptr) return magenta;

        x = clamp(x, 0, image_width);
        y = clamp(y, 0, image_height);

        return data + y * bytes_per_line() + x * bytes_per_pixel;
    }

private:
    const int bytes_per_pixel = 3;
    unsigned char *data;
    int image_width, image_height;
    int bytes_per_line() const { return image_width * bytes_per_pixel; }

    static int clamp(int x, int low, int high) {
        if (x < low) return low;
        if (x < high) return x;
        return high - 1;
    }
};

} // namespace rt

#ifdef _MSC_VER
    #pragma warning (pop)
#endif