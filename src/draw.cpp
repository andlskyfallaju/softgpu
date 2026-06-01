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

// ── Gouraud-shaded textured triangle (Chapter 6) ────────────────────────────

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
            P.z = t0.z*bc.x + t1.z*bc.y + t2.z*bc.z;
            int idx = int(P.x) + int(P.y)*image.get_width();
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                float ity = ity0*bc.x + ity1*bc.y + ity2*bc.z;
                ity = std::max(0.f, std::min(1.f, ity));

                float u = uv0.x*bc.x + uv1.x*bc.y + uv2.x*bc.z;
                float v = uv0.y*bc.x + uv1.y*bc.y + uv2.y*bc.z;
                Color texColor = tex.sample(u, v);

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

// ── Normal-mapped + specular triangle (Chapter 7) ───────────────────────────

void triangle_nm(Vec3f t0, Vec3f t1, Vec3f t2,
                 Vec2f uv0, Vec2f uv1, Vec2f uv2,
                 Vec3f n0, Vec3f n1, Vec3f n2,
                 float *zbuffer, Image &image,
                 Texture &diffuse, Texture &normalMap, Texture &specMap,
                 Vec3f light_dir, float ambient, float spec_power) {
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

    // Precompute edge vectors for tangent basis (Darboux frame)
    Vec3f edge1 = t1 - t0;
    Vec3f edge2 = t2 - t0;
    Vec2f duv1(uv1.x - uv0.x, uv1.y - uv0.y);
    Vec2f duv2(uv2.x - uv0.x, uv2.y - uv0.y);

    float det = duv1.x * duv2.y - duv2.x * duv1.y;
    // If the triangle is degenerate in UV space, skip tangent computation
    bool has_tangent = std::abs(det) > 1e-6f;

    Vec3f T, B; // tangent, bitangent
    if (has_tangent) {
        float inv_det = 1.f / det;
        T = Vec3f(
            inv_det * (duv2.y * edge1.x - duv1.y * edge2.x),
            inv_det * (duv2.y * edge1.y - duv1.y * edge2.y),
            inv_det * (duv2.y * edge1.z - duv1.y * edge2.z)
        );
        B = Vec3f(
            inv_det * (-duv2.x * edge1.x + duv1.x * edge2.x),
            inv_det * (-duv2.x * edge1.y + duv1.x * edge2.y),
            inv_det * (-duv2.x * edge1.z + duv1.x * edge2.z)
        );
        T.normalize();
        B.normalize();
    }

    Vec3f P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc = barycentric(t0, t1, t2, P);
            if (bc.x<0 || bc.y<0 || bc.z<0) continue;

            P.z = t0.z*bc.x + t1.z*bc.y + t2.z*bc.z;
            int idx = int(P.x) + int(P.y)*image.get_width();
            if (zbuffer[idx] >= P.z) continue;
            zbuffer[idx] = P.z;

            // Interpolate UV
            float u = uv0.x*bc.x + uv1.x*bc.y + uv2.x*bc.z;
            float v = uv0.y*bc.x + uv1.y*bc.y + uv2.y*bc.z;

            // Interpolate the geometric normal
            Vec3f N(
                n0.x*bc.x + n1.x*bc.y + n2.x*bc.z,
                n0.y*bc.x + n1.y*bc.y + n2.y*bc.z,
                n0.z*bc.x + n1.z*bc.y + n2.z*bc.z
            );
            N.normalize();

            // Sample the tangent-space normal map
            Vec3f pixel_normal = N; // fallback
            if (normalMap.is_loaded() && has_tangent) {
                Color nm = normalMap.sample(u, v);
                // Decode from [0,255] to [-1,1]
                Vec3f ts_normal(
                    nm.r / 255.f * 2.f - 1.f,
                    nm.g / 255.f * 2.f - 1.f,
                    nm.b / 255.f * 2.f - 1.f
                );
                // Transform tangent-space normal to world space via TBN matrix
                pixel_normal = Vec3f(
                    T.x * ts_normal.x + B.x * ts_normal.y + N.x * ts_normal.z,
                    T.y * ts_normal.x + B.y * ts_normal.y + N.y * ts_normal.z,
                    T.z * ts_normal.x + B.z * ts_normal.y + N.z * ts_normal.z
                );
                pixel_normal.normalize();
            }

            // Diffuse lighting (Lambertian)
            float diff = std::max(0.f, pixel_normal * light_dir);

            // Specular highlights (Blinn-Phong)
            float spec = 0.f;
            if (specMap.is_loaded()) {
                // For a simple viewer at infinity looking down -Z,
                // the half vector approximation:
                Vec3f view_dir(0, 0, 1);
                Vec3f half_dir = Vec3f(
                    light_dir.x + view_dir.x,
                    light_dir.y + view_dir.y,
                    light_dir.z + view_dir.z
                );
                half_dir.normalize();
                float spec_angle = std::max(0.f, pixel_normal * half_dir);
                Color sc = specMap.sample(u, v);
                float spec_strength = sc.r / 255.f; // use red channel
                spec = spec_strength * std::pow(spec_angle, spec_power);
            }

            // Final intensity = ambient + diffuse + specular
            float intensity = ambient + diff + spec;
            if (intensity > 1.f) intensity = 1.f;

            // Sample diffuse texture and apply lighting
            Color texColor = diffuse.sample(u, v);
            Color c = {
                (uint8_t)(texColor.r * intensity),
                (uint8_t)(texColor.g * intensity),
                (uint8_t)(texColor.b * intensity),
                255
            };
            image.set(P.x, P.y, c);
        }
    }
}
