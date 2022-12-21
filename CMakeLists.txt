cmake_minimum_required(VERSION 3.15)
project(cv_make_png)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(main Main.cpp)
target_compile_options(main PUBLIC -O2)
target_link_libraries(main ${OpenCV_LIBS})
