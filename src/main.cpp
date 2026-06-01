#include <iostream>
#include <cstdlib>
#include <limits>
#include "image.h"
#include "draw.h"
#include "model.h"
#include "geometry.h"

const int width  = 800;
const int height = 800;

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = 255.f/2.f;
    
    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = 255.f/2.f;
    return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = (up^z).normalize();
    Vec3f y = (z^x).normalize();
    Matrix Minv = Matrix::identity(4);
    Matrix Tr   = Matrix::identity(4);
    for (int i=0; i<3; i++) {
        Minv[0][i] = x.raw[i];
        Minv[1][i] = y.raw[i];
        Minv[2][i] = z.raw[i];
        Tr[i][3] = -center.raw[i];
    }
    return Minv*Tr;
}

Matrix projection(float camera_z) {
    Matrix Projection = Matrix::identity(4);
    Projection[3][2] = -1.f/camera_z;
    return Projection;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: softgpu <obj_file> [diffuse.tga] [normal_map.tga] [spec_map.tga]" << std::endl;
        return 1;
    }

    std::cout << "Loading model " << argv[1] << "..." << std::endl;
    Model *model = new Model(argv[1]);
    Image image(width, height);

    // Load textures
    Texture diffuse, normalMap, specMap;
    if (argc >= 3) {
        if (diffuse.load(argv[2]))
            std::cout << "Loaded diffuse: " << argv[2] << " (" << diffuse.get_width() << "x" << diffuse.get_height() << ")" << std::endl;
        else
            std::cerr << "WARNING: Failed to load diffuse " << argv[2] << std::endl;
    }
    if (argc >= 4) {
        if (normalMap.load(argv[3]))
            std::cout << "Loaded normal map: " << argv[3] << " (" << normalMap.get_width() << "x" << normalMap.get_height() << ")" << std::endl;
        else
            std::cerr << "WARNING: Failed to load normal map " << argv[3] << std::endl;
    }
    if (argc >= 5) {
        if (specMap.load(argv[4]))
            std::cout << "Loaded specular map: " << argv[4] << " (" << specMap.get_width() << "x" << specMap.get_height() << ")" << std::endl;
        else
            std::cerr << "WARNING: Failed to load specular map " << argv[4] << std::endl;
    }

    float *zbuffer = new float[width*height];
    for (int i=0; i<width*height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    Vec3f light_dir(0, 0, 1);
    light_dir.normalize();
    Vec3f camera(0, 0, 3);

    Matrix ModelView  = lookat(camera, Vec3f(0,0,0), Vec3f(0,1,0));
    Matrix Projection = projection(camera.z);
    Matrix ViewPort   = viewport(width/8, height/8, width*3/4, height*3/4);

    bool use_normal_map = normalMap.is_loaded();

    std::cout << "Rendering " << model->nfaces() << " triangles"
              << (use_normal_map ? " (normal-mapped + specular)" : " (Gouraud)") 
              << "..." << std::endl;

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<Vec3i> face = model->face_data(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec3f normals[3];
        Vec2f uvs[3];

        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j].raw[0]);
            screen_coords[j] = m2vec(ViewPort * Projection * ModelView * vec2m(v));
            world_coords[j]  = v;
            normals[j] = model->norm(face[j].raw[2]);
            normals[j].normalize();
            uvs[j] = model->uv(face[j].raw[1]);
        }

        if (use_normal_map) {
            // Quick back-face test using average normal
            Vec3f avg_n = Vec3f(
                (normals[0].x + normals[1].x + normals[2].x) / 3.f,
                (normals[0].y + normals[1].y + normals[2].y) / 3.f,
                (normals[0].z + normals[1].z + normals[2].z) / 3.f
            );
            avg_n.normalize();
            if (avg_n * light_dir < -0.1f) continue; // back face, skip

            triangle_nm(screen_coords[0], screen_coords[1], screen_coords[2],
                         uvs[0], uvs[1], uvs[2],
                         normals[0], normals[1], normals[2],
                         zbuffer, image,
                         diffuse, normalMap, specMap,
                         light_dir, 0.05f, 16.f);
        } else {
            // Fallback: Gouraud shading (Chapter 6 path)
            float intensity[3];
            for (int j=0; j<3; j++) {
                intensity[j] = std::max(0.f, normals[j] * light_dir);
            }
            if (intensity[0]>0 || intensity[1]>0 || intensity[2]>0) {
                triangle(screen_coords[0], screen_coords[1], screen_coords[2],
                         uvs[0], uvs[1], uvs[2],
                         intensity[0], intensity[1], intensity[2],
                         zbuffer, image, diffuse);
            }
        }
    }

    if (image.write_png("output.png")) {
        std::cout << "Successfully saved output.png" << std::endl;
    } else {
        std::cout << "Failed to save output.png" << std::endl;
    }

    delete[] zbuffer;
    delete model;
    return 0;
}
