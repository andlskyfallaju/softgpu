#pragma once
#include <vector>
#include "geometry.h"

class Model {
private:
    std::vector<Vec3f> verts_;
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;
    std::vector<std::vector<Vec3i>> faces_; // vertex/uv/normal
public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f vert(int i);
    Vec3f norm(int i);
    Vec2f uv(int i);
    std::vector<int> face(int idx); // backwards compatibility for just vertices
    std::vector<Vec3i> face_data(int idx); // full face data
};
