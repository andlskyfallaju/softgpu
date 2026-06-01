#pragma once
#include <vector>
#include <string>
#include <cstdint>

struct Color {
    uint8_t r, g, b, a;
};

class Image {
public:
    Image(int width, int height);

    void set(int x, int y, const Color& color);
    Color get(int x, int y) const;
    
    bool write_png(const std::string& filename) const;
    
    int get_width() const { return width; }
    int get_height() const { return height; }
    
    void clear(const Color& color);

private:
    int width;
    int height;
    std::vector<uint8_t> data;
};

// Texture loaded from disk (TGA/PNG/JPG via stb_image)
class Texture {
public:
    Texture();
    ~Texture();

    bool load(const std::string& filename);

    // Sample the texture at normalized UV coordinates [0,1]
    // V is flipped to match TGA bottom-left origin vs render top-left
    Color sample(float u, float v) const;

    int get_width()  const { return width; }
    int get_height() const { return height; }
    bool is_loaded() const { return data != nullptr; }

private:
    unsigned char* data;
    int width, height, channels;
};
