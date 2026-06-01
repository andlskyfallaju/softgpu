#include "image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include <algorithm>
#include <cstring>

// ── Image ────────────────────────────────────────────────────────────────────

Image::Image(int w, int h) : width(w), height(h), data(w * h * 4, 0) {}

void Image::set(int x, int y, const Color& color) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    // Flip Y: origin at bottom-left (tinyrenderer convention)
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

// ── Texture ──────────────────────────────────────────────────────────────────

Texture::Texture() : data(nullptr), width(0), height(0), channels(0) {}

Texture::~Texture() {
    if (data) stbi_image_free(data);
}

bool Texture::load(const std::string& filename) {
    data = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    if (!data) {
        return false;
    }
    return true;
}

Color Texture::sample(float u, float v) const {
    if (!data) return {255, 0, 255, 255}; // magenta = missing texture

    // Clamp UV to [0,1]
    u = std::max(0.f, std::min(1.f, u));
    v = std::max(0.f, std::min(1.f, v));

    // OBJ UV: v=0 is bottom of texture. stb_image row 0 = top.
    // So flip v to map correctly.
    v = 1.f - v;

    int tx = (int)(u * (width  - 1));
    int ty = (int)(v * (height - 1));

    int index = (ty * width + tx) * 4;
    return { data[index+0], data[index+1], data[index+2], data[index+3] };
}
