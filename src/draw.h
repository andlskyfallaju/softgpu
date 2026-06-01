#pragma once
#include "image.h"
#include "geometry.h"

void line(int x0, int y0, int x1, int y1, Image& image, const Color& color);

// Triangle with texture, Gouraud shading (no normal map).
void triangle(Vec3f t0, Vec3f t1, Vec3f t2,
              Vec2f uv0, Vec2f uv1, Vec2f uv2,
              float ity0, float ity1, float ity2,
              float *zbuffer, Image &image, Texture &tex);

// Triangle with texture, per-pixel normal mapping, and specular highlights.
// n0/n1/n2 are per-vertex normals (used to build the tangent-space basis).
// normalMap stores tangent-space normals. specMap stores specular intensity.
// light_dir must be normalized.
void triangle_nm(Vec3f t0, Vec3f t1, Vec3f t2,
                 Vec2f uv0, Vec2f uv1, Vec2f uv2,
                 Vec3f n0, Vec3f n1, Vec3f n2,
                 float *zbuffer, Image &image,
                 Texture &diffuse, Texture &normalMap, Texture &specMap,
                 Vec3f light_dir, float ambient = 0.05f, float spec_power = 16.f);
