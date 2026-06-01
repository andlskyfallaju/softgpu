#include "draw.h"
#include <cmath>
#include <algorithm>

void line(int x0, int y0, int x1, int y1, Image& image, const Color& color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    int y_step = (y1 > y0) ? 1 : -1;
    for (int x = x0; x <= x1; x++) {
        if (steep) image.set(y, x, color);
        else image.set(x, y, color);
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

void triangle(Vec3f t0, Vec3f t1, Vec3f t2,
              Vec2f uv0, Vec2f uv1, Vec2f uv2,
              float ity0, float ity1, float ity2,
              float *zbuffer, Image &image, Texture &tex) {
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
            Vec3f bc = barycentric(t0, t1, t2, P);
            if (bc.x<0 || bc.y<0 || bc.z<0) continue;

            // Interpolate Z
            P.z = t0.z*bc.x + t1.z*bc.y + t2.z*bc.z;
            int idx = int(P.x) + int(P.y)*image.get_width();
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;

                // Interpolate intensity (Gouraud)
                float ity = ity0*bc.x + ity1*bc.y + ity2*bc.z;
                if (ity > 1.f) ity = 1.f;
                if (ity < 0.f) ity = 0.f;

                // Interpolate UV and sample texture
                float u = uv0.x*bc.x + uv1.x*bc.y + uv2.x*bc.z;
                float v = uv0.y*bc.x + uv1.y*bc.y + uv2.y*bc.z;
                Color texColor = tex.sample(u, v);

                // Apply lighting to texture color
                Color c = {
                    (uint8_t)(texColor.r * ity),
                    (uint8_t)(texColor.g * ity),
                    (uint8_t)(texColor.b * ity),
                    255
                };
                image.set(P.x, P.y, c);
            }
        }
    }
}
