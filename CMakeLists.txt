cmake_minimum_required(VERSION 3.15)
project(cv_make_png)
set(CMAKE_CXX_STANDARD 20)

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# lpthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# main
add_compile_definitions(MAX_THREAD=${MAX_THREAD})
add_executable(main main.cpp)
target_compile_options(main PUBLIC -O2)

# link
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(main Threads::Threads)
