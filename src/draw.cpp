#include "draw.h"
#include <cmath>
#include <algorithm>

void line(int x0, int y0, int x1, int y1, Image& image, const Color& color) {
    bool steep = false;
    
    // If the line is steep, we transpose the image coordinates
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    
    // Make sure we draw from left to right
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    
    int dx = x1 - x0;
    int dy = y1 - y0;
    
    // The error accumulator. We scale it up by 2*dx to avoid floating point math.
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    
    int y = y0;
    int y_step = (y1 > y0) ? 1 : -1;
    
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color); // if transposed, de-transpose
        } else {
            image.set(x, y, color);
        }
        
        error2 += derror2;
        if (error2 > dx) {
            y += y_step;
            error2 -= dx * 2;
        }
    }
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i].raw[0] = C.raw[i]-A.raw[i];
        s[i].raw[1] = B.raw[i]-A.raw[i];
        s[i].raw[2] = A.raw[i]-P.raw[i];
    }
    Vec3f u = s[0] ^ s[1];
    if (std::abs(u.z)>1e-2) 
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); 
}

void triangle(Vec3f t0, Vec3f t1, Vec3f t2, float *zbuffer, Image &image, const Color& color) {
    Vec2i bboxmin(image.get_width()-1,  image.get_height()-1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width()-1, image.get_height()-1);
    Vec3f pts[3] = {t0, t1, t2};
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0, std::min(bboxmin.x, (int)pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, (int)pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, (int)pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, (int)pts[i].y));
    }
    Vec3f P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc_screen  = barycentric(t0, t1, t2, P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = 0;
            P.z += t0.z * bc_screen.x;
            P.z += t1.z * bc_screen.y;
            P.z += t2.z * bc_screen.z;
            int idx = int(P.x) + int(P.y)*image.get_width();
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}
