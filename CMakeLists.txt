cmake_minimum_required(VERSION 3.15)
project(cv_make_png)
set(CMAKE_CXX_STANDARD 17)
set(CUDA_STANDARD 17)

# options
option(BUILD_CPU "build for cpu?" OFF)
option(BUILD_GPU "build for gpu?" OFF)

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# set renderer
include_directories(${PROJECT_SOURCE_DIR}/render/${RENDERER})

# utils
include_directories(${PROJECT_SOURCE_DIR}/include)

# CPU
if(${BUILD_CPU})
	# lpthread
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)

	# main_cpu
	add_compile_definitions(MAX_THREAD=${MAX_THREAD})
	add_executable(main main.cpp)
	target_compile_options(main PUBLIC -march=native -O2)

	# link_cpu
	target_link_libraries(main ${OpenCV_LIBS})
	target_link_libraries(main Threads::Threads)
endif()

# GPU
if(${BUILD_GPU})
	# CUDA
	find_package(CUDA REQUIRED)
	message("-- CUDA --")
	message("Version: " ${CUDA_VERSION})
	message("Library: " ${CUDA_CUDA_LIBRARY})
	message("Runtime: " ${CUDA_CUDART_LIBRARY})

	# main_gpu
	add_compile_definitions(MAX_THREAD=${MAX_THREAD})
	set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -O2 --expt-extended-lambda --expt-relaxed-constexpr)
	cuda_add_executable(main_gpu main.cu)

	# link_gpu
	target_link_libraries(main_gpu ${OpenCV_LIBS})
endif()
