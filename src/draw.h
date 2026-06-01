#pragma once
#include "image.h"
#include "geometry.h"

void line(int x0, int y0, int x1, int y1, Image& image, const Color& color);

// Triangle with Z-buffer, Gouraud shading, and texture mapping.
// uv0/uv1/uv2 are per-vertex texture coordinates in [0,1].
// ity0/ity1/ity2 are per-vertex lighting intensities in [0,1].
// If tex is not loaded, falls back to flat white shading.
void triangle(Vec3f t0, Vec3f t1, Vec3f t2,
              Vec2f uv0, Vec2f uv1, Vec2f uv2,
              float ity0, float ity1, float ity2,
              float *zbuffer, Image &image, Texture &tex);
