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
    
    // Clear image to a specific color
    void clear(const Color& color);

private:
    int width;
    int height;
    std::vector<uint8_t> data;
};
