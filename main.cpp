#include <iostream>
#include <cstdio>
#include <string>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cmath>
#include <thread>
#include <vector>
#include <atomic>
#include <opencv2/opencv.hpp>
#include <sys/ioctl.h>
#include <unistd.h>

#include "blend.hpp"

// -+-+-+-+-+-+-+-+-+-+- //
//        Utility        //
// -+-+-+-+-+-+-+-+-+-+- //

std::string zero_ume(int i, int width = 6){
	std::ostringstream ss;
	ss << std::setfill('0') << std::right << std::setw(width) << i;
	return ss.str();
}

void progress_bar(long long int numerator, long long int denominator){
	struct winsize winsz;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz);
	++numerator;
	std::ostringstream ss;
	ss << "\e[2K\r |";
	int max = winsz.ws_col - 20 - 2*std::log10(denominator);
	for(int i=0; i<max; ++i)
		ss << (numerator* max >= i * denominator ? "\e[42m" : "\e[41m") << " ";
	ss << "\e[0m| "
		<< numerator << " / " << denominator
		<< " (" << int(double(numerator*100) / denominator) << '%' << ")";
	std::cerr << ss.str() << std::flush;
};


// -+-+-+-+-+-+-+-+-+-+- //
//       Main Loop       //
// -+-+-+-+-+-+-+-+-+-+- //

struct Status {
	int frame;
	float fps;
	float time;
	float duration;
	int height;
	int width;
};

namespace renderer {
void init(Status& status);
void render(cv::Mat& img, const Status status);
}

int main(int argc, char *argv[]){
	const float fps_      = (argc > 1 ? std::stof(argv[1]) :   30);  // デフォルト: 30 fps
	const int   width_    = (argc > 2 ? std::stoi(argv[2]) : 1920);  // デフォルト: 1920 px
	const int   height_   = (argc > 3 ? std::stoi(argv[3]) : 1080);  // デフォルト: 1080 px

	Status status{ 0, fps_, 0, 0, height_, width_ };
	renderer::init(status);

	std::cout << "fps: "       << status.fps      << std::endl;
	std::cout << "duration: "  << status.duration << std::endl;
	std::cout << "width: "     << status.width    << std::endl;
	std::cout << "height: "    << status.height   << std::endl;
	std::cout << "max thread:" <<  MAX_THREAD << std::endl;

	std::atomic_int done_frame_cnt{0};
	int total_frame_cnt = status.fps * status.duration;
	// 数回に分けて，スレッドの塊を立ち上げていく
	for(int thread_phase = 0; thread_phase*MAX_THREAD < total_frame_cnt; ++thread_phase){
		std::vector<std::thread> threads;
		// スレッドを立ち上げていく
		for(int frame = thread_phase*MAX_THREAD; frame < (thread_phase+1)*MAX_THREAD && frame < total_frame_cnt; ++frame){
			threads.emplace_back([=, &done_frame_cnt]{
				float time = float(frame) / status.fps;
				cv::Mat img = cv::Mat::zeros(cv::Size(status.width, status.height), CV_MAKE_TYPE(CV_8U, 4));

				Status current_status = status;
				current_status.frame = frame;
				current_status.time  = time;
				renderer::render(img, current_status);

				std::ostringstream file_name;
				file_name << "png/out_" << zero_ume(frame) << ".png";
				cv::imwrite(file_name.str(), img);

				progress_bar(done_frame_cnt++, total_frame_cnt);
			});
		}
		// スレッドの塊を待つ
		for(auto& th : threads)
			th.join();
	}
	std::cerr << std::endl;
}


// -+-+-+-+-+-+-+-+-+-+- //
//        Include        //
// -+-+-+-+-+-+-+-+-+-+- //

#include "render/square_transition.hpp"
