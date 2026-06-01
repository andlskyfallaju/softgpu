#include "image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"
#include <stdexcept>

Image::Image(int w, int h) : width(w), height(h), data(w * h * 4, 0) {
}

void Image::set(int x, int y, const Color& color) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    
    // Flip Y to match standard cartesian coordinates (0,0 at bottom-left)
    // Tinyrenderer assumes origin is at bottom left
    int flipped_y = height - 1 - y;
    
    int index = (flipped_y * width + x) * 4;
    data[index + 0] = color.r;
    data[index + 1] = color.g;
    data[index + 2] = color.b;
    data[index + 3] = color.a;
}

Color Image::get(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return {0,0,0,0};
    int flipped_y = height - 1 - y;
    int index = (flipped_y * width + x) * 4;
    return { data[index+0], data[index+1], data[index+2], data[index+3] };
}

void Image::clear(const Color& color) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 4;
            data[index + 0] = color.r;
            data[index + 1] = color.g;
            data[index + 2] = color.b;
            data[index + 3] = color.a;
        }
    }
}

bool Image::write_png(const std::string& filename) const {
    int success = stbi_write_png(filename.c_str(), width, height, 4, data.data(), width * 4);
    return success != 0;
}
