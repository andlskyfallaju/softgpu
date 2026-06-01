#include <iostream>
#include <cstdlib>
#include <limits>
#include "image.h"
#include "draw.h"
#include "model.h"
#include "geometry.h"

const int width  = 800;
const int height = 800;

int main(int argc, char** argv) {
    if (argc == 2) {
        std::cout << "Loading model " << argv[1] << "..." << std::endl;
        Model *model = new Model(argv[1]);
        Image image(width, height);
        
        // Allocate and initialize Z-buffer
        float *zbuffer = new float[width*height];
        for (int i=0; i<width*height; i++) {
            zbuffer[i] = -std::numeric_limits<float>::max();
        }
        
        // Light direction for flat shading
        Vec3f light_dir(0,0,-1);
        
        std::cout << "Rendering " << model->nfaces() << " triangles..." << std::endl;
        for (int i=0; i<model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3f screen_coords[3];
            Vec3f world_coords[3];
            for (int j=0; j<3; j++) {
                Vec3f v = model->vert(face[j]);
                screen_coords[j] = Vec3f(int((v.x+1.)*width/2. + .5), int((v.y+1.)*height/2. + .5), v.z);
                world_coords[j]  = v;
            }
            
            // Calculate face normal
            Vec3f n = (world_coords[2]-world_coords[0]) ^ (world_coords[1]-world_coords[0]);
            n.normalize();
            
            // Calculate intensity based on light direction
            float intensity = n * light_dir;
            
            // Only draw if the face is pointing towards the light (and camera)
            if (intensity > 0) {
                Color c = {(uint8_t)(intensity*255), (uint8_t)(intensity*255), (uint8_t)(intensity*255), 255};
                triangle(screen_coords[0], screen_coords[1], screen_coords[2], zbuffer, image, c);
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
