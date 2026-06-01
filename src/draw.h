#pragma once
#include "image.h"
#include "geometry.h"

void line(int x0, int y0, int x1, int y1, Image& image, const Color& color);
void triangle(Vec3f t0, Vec3f t1, Vec3f t2, float ity0, float ity1, float ity2, float *zbuffer, Image &image, const Color& color);
