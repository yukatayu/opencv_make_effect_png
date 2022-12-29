cmake_minimum_required(VERSION 3.15)
project(cv_make_png)
set(CMAKE_CXX_STANDARD 17)
set(CUDA_STANDARD 17)

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# CUDA
find_package(CUDA REQUIRED)
message("-- CUDA --")
message("Version: " ${CUDA_VERSION})
message("Library: " ${CUDA_CUDA_LIBRARY})
message("Runtime: " ${CUDA_CUDART_LIBRARY})

# set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -arch ${CUDA_ARCH} --expt-extended-lambda --expt-relaxed-constexpr")
# set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O2 --expt-extended-lambda --expt-relaxed-constexpr")

# lpthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# set renderer
include_directories(${PROJECT_SOURCE_DIR}/render/${RENDERER})

# utils
include_directories(${PROJECT_SOURCE_DIR}/include)

# main_cpu
add_compile_definitions(MAX_THREAD=${MAX_THREAD})
cuda_add_executable(main main.cpp)
target_compile_options(main PUBLIC -march=native -O2)

# link_cpu
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(main Threads::Threads)

# main_gpu
add_compile_definitions(MAX_THREAD=${MAX_THREAD})

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -O2 --expt-extended-lambda --expt-relaxed-constexpr)

cuda_add_executable(main_gpu main.cu)
# target_compile_options(main_gpu PUBLIC -march=native -O2 --expt-relaxed-constexpr)

# link_gpu
target_link_libraries(main_gpu ${OpenCV_LIBS})
