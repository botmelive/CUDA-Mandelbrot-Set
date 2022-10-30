cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(mandelbrot_set LANGUAGES CXX CUDA)

include_directories(
        "libs/glad/include/"
	"libs/glew/include/"
        "libs/freeglut/include"
        "includes/"
)

find_package(CUDAToolkit)
add_subdirectory(libs/freeglut/)
add_subdirectory(libs/glew)

add_executable(mandelbrot 
src/main.cpp
src/kernel.cu
)

target_compile_features(mandelbrot PUBLIC cxx_std_17)
 
set_target_properties( mandelbrot
                       PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

if(WIN32)
        target_link_libraries(mandelbrot PRIVATE libglew_static freeglut_static opengl32)
endif(WIN32)

if(UNIX)
        target_link_libraries(mandelbrot libglew_static freeglut_static GL)
endif(UNIX)