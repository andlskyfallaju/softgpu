#pragma once
#include <cmath>

template <class t> struct Vec2 {
    union {
        struct {t x, y;};
        t raw[2];
    };
    Vec2() : x(0), y(0) {}
    Vec2(t _x, t _y) : x(_x), y(_y) {}
    inline Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(x+V.x, y+V.y); }
    inline Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(x-V.x, y-V.y); }
    inline Vec2<t> operator *(float f)          const { return Vec2<t>(x*f, y*f); }
};

template <class t> struct Vec3 {
    union {
        struct {t x, y, z;};
        t raw[3];
    };
    Vec3() : x(0), y(0), z(0) {}
    Vec3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    inline Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
    inline Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
    inline Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
    inline Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
    inline t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
    float norm () const { return std::sqrt(x*x+y*y+z*z); }
    Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

#include <vector>
#include <iostream>

class Matrix {
    std::vector<std::vector<float> > m;
    int rows, cols;
public:
    Matrix(int r=4, int c=4);
    int nrows();
    int ncols();
    static Matrix identity(int dimensions);
    std::vector<float>& operator[](const int i);
    const std::vector<float>& operator[](const int i) const;
    Matrix operator*(const Matrix& a) const;
    Matrix transpose() const;
    Matrix inverse() const;
    friend std::ostream& operator<<(std::ostream& s, Matrix& m);
};

Matrix vec2m(Vec3f v);
Vec3f m2vec(Matrix m);
