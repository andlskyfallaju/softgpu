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
    if (argc == 2) {
        std::cout << "Loading model " << argv[1] << "..." << std::endl;
        Model *model = new Model(argv[1]);
        Image image(width, height);
        
        float *zbuffer = new float[width*height];
        for (int i=0; i<width*height; i++) {
            zbuffer[i] = -std::numeric_limits<float>::max();
        }
        
        Vec3f light_dir(0,0,-1);
        Vec3f camera(0,0,3);
        
        Matrix ModelView  = lookat(camera, Vec3f(0,0,0), Vec3f(0,1,0));
        Matrix Projection = projection(camera.z);
        Matrix ViewPort   = viewport(width/8, height/8, width*3/4, height*3/4);
        
        std::cout << "Rendering " << model->nfaces() << " triangles..." << std::endl;
        for (int i=0; i<model->nfaces(); i++) {
            std::vector<Vec3i> face = model->face_data(i);
            Vec3f screen_coords[3];
            Vec3f world_coords[3];
            float intensity[3];
            
            for (int j=0; j<3; j++) {
                Vec3f v = model->vert(face[j].raw[0]);
                screen_coords[j] = m2vec(ViewPort * Projection * ModelView * vec2m(v));
                world_coords[j]  = v;
                
                // Calculate intensity per vertex using vertex normals from OBJ
                Vec3f n = model->norm(face[j].raw[2]);
                n.normalize();
                intensity[j] = std::max(0.f, n * light_dir);
            }
            
            // Only draw if at least one vertex is facing the light
            if (intensity[0]>0 || intensity[1]>0 || intensity[2]>0) {
                Color base_color = {255, 255, 255, 255}; // Base color is white
                triangle(screen_coords[0], screen_coords[1], screen_coords[2], 
                         intensity[0], intensity[1], intensity[2], 
                         zbuffer, image, base_color);
            }
        }

        if (image.write_png("output.png")) {
            std::cout << "Successfully saved output.png" << std::endl;
        } else {
            std::cout << "Failed to save output.png" << std::endl;
        }
        
        delete[] zbuffer;
        delete model;
    } else {
        std::cerr << "Usage: softgpu <obj_file>" << std::endl;
        return 1;
    }
    return 0;
}
