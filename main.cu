#include <string>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cmath>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <opencv2/opencv.hpp>
#include <cuda_runtime_api.h>
#include "util.hpp"
#include "protocol.hpp"

#define cudaAssert(ans) { gpuAssert_impl((ans), __FILE__, __LINE__); }
inline void cudaCheck(){
	cudaError_t err = cudaGetLastError();
	if(err != cudaSuccess)
		std::cerr << "CUDA error: " << cudaGetErrorString(err) << std::endl;
}
inline void gpuAssert_impl(cudaError_t code, const char *file, int line, bool abort=true){
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

// -+-+-+-+-+-+-+-+-+-+- //
//       Main Loop       //
// -+-+-+-+-+-+-+-+-+-+- //

__global__ void clearGpuMemory(unsigned char* ptr, int size, int parallel){
	const int idx = blockIdx.x * blockDim.x + threadIdx.x;
	const int lb =  idx    * parallel;
	int ub = (idx+1) * parallel;
	ub = ub < size ? ub : size;
	for(int i = lb; i < ub; ++i)
		ptr[i] = 0;
}

__device__ void invoke_render_impl(unsigned char* img, const Status status, int offset){
	const int idx = blockIdx.x * blockDim.x + threadIdx.x + offset;
	const int y = idx / status.width;
	const int x = idx % status.width;
	if(y < status.height){
		unsigned char* ptr = img + (y * status.width + x) * 4;
		RGBA col = renderer_gpu::render(img, status, x, y);
		ptr[0] = col.b;
		ptr[1] = col.g;
		ptr[2] = col.r;
		ptr[3] = col.a;
	}
}

__global__ void invoke_render(unsigned char* img, const Status status, int offset){
	invoke_render_impl(img, status, offset);
}

int main(int argc, char *argv[]){
	const float fps_      = (argc > 1 ? std::stof(argv[1]) :   30);  // デフォルト: 30 fps
	const int   width_    = (argc > 2 ? std::stoi(argv[2]) : 1920);  // デフォルト: 1920 px
	const int   height_   = (argc > 3 ? std::stoi(argv[3]) : 1080);  // デフォルト: 1080 px

	Status status{ 0, fps_, 0, 0, height_, width_ };
	renderer_cpu::init(status);

	std::cout << "fps: "       << status.fps      << std::endl;
	std::cout << "duration: "  << status.duration << std::endl;
	std::cout << "width: "     << status.width    << std::endl;
	std::cout << "height: "    << status.height   << std::endl;

	std::atomic_int done_frame_cnt{0};
	int total_frame_cnt = status.fps * status.duration;

	// GPUメモリ確保
	unsigned char* device_img_raw;
	cudaAssert(cudaMalloc(reinterpret_cast<void**>(&device_img_raw), status.width*status.height*4*sizeof(unsigned char)));
	std::shared_ptr<unsigned char> device_img(device_img_raw, cudaFree);
	clearGpuMemory<<<status.width/4+1,status.height/4+1>>>(device_img.get(), status.width*status.height*4, 64);
	cudaCheck();
	
	// CPUメモリ確保
	cv::Mat img = cv::Mat::zeros(cv::Size(status.width, status.height), CV_MAKE_TYPE(CV_8U, 4));
	
	// メインループ
	for(int frame = 0; frame < total_frame_cnt; ++frame){
		float time = float(frame) / status.fps;

		Status current_status = status;
		current_status.frame = frame;
		current_status.time  = time;
		for(int i=0; i*1024*1024 < status.height * status.width; ++i)
			invoke_render<<<1024,1024>>>(device_img_raw, current_status, i*1024*1024);
		cudaCheck();

		cudaAssert(cudaMemcpy(img.data, device_img.get(), status.width*status.height*4, cudaMemcpyDeviceToHost));
		cudaDeviceSynchronize();

		std::ostringstream file_name;
		file_name << "png/out_" << zero_ume(frame) << ".png";
		cv::imwrite(file_name.str(), img);

		progress_bar(frame, total_frame_cnt);
	}
	std::cerr << std::endl;
}


// -+-+-+-+-+-+-+-+-+-+- //
//        Include        //
// -+-+-+-+-+-+-+-+-+-+- //

#include "main.hpp"
