cmake_minimum_required(VERSION 3.15)
project(softgpu VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the include directory so it can find stb_image_write.h
include_directories(include)

# Adding the main executable
add_executable(softgpu 
    src/main.cpp
    src/image.cpp
    src/draw.cpp
    src/model.cpp
    src/geometry.cpp
)

# Later we will add target_include_directories and target_compile_features for AVX-512 here
