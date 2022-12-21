#include <iostream>
#include <cstdio>
#include <string>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <sys/ioctl.h>
#include <unistd.h>


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
	std::cerr << "\e[2K\r |";
	int max = winsz.ws_col - 20 - 2*std::log10(denominator);
	for(int i=0; i<max; ++i)
		std::cerr << (numerator* max >= i * denominator ? "\e[42m" : "\e[41m") << " ";
	std::cerr << "\e[0m| "
		<< numerator << " / " << denominator
		<< " (" << int(double(numerator*100) / denominator) << '%' << ")"
		<< std::flush;
};


// -+-+-+-+-+-+-+-+-+-+- //
//       Main Loop       //
// -+-+-+-+-+-+-+-+-+-+- //

struct Status {
	int frame;
	float fps;
	float time;
	int duration;
	int height;
	int width;
};

void init(Status& status);
void render(cv::Mat& img, const Status status);

int main(int argc, char *argv[]){
	const float fps_      = (argc > 1 ? std::stof(argv[1]) :   30);  // デフォルト: 30 fps
	const int   width_    = (argc > 2 ? std::stoi(argv[2]) : 1920);  // デフォルト: 1920 px
	const int   height_   = (argc > 3 ? std::stoi(argv[3]) : 1080);  // デフォルト: 1080 px

	Status status{ 0, fps_, 0, 0, height_, width_ };
	init(status);

	std::cout << "fps: "      << status.fps      << std::endl;
	std::cout << "duration: " << status.duration << std::endl;
	std::cout << "width: "    << status.width    << std::endl;
	std::cout << "height: "   << status.height   << std::endl;

	for(int frame = 0; frame < status.fps * status.duration; ++frame){
		progress_bar(frame, status.fps * status.duration);
		float time = float(frame) / status.fps;
		cv::Mat img = cv::Mat::zeros(cv::Size(status.width, status.height), CV_MAKE_TYPE(CV_8U, 4));

		status.frame = frame;
		status.time  = time;
		render(img, status);

		std::ostringstream file_name;
		file_name << "png/out_" << zero_ume(frame) << ".png";
		cv::imwrite(file_name.str(), img);
	}
	std::cerr << std::endl;
}


// -+-+-+-+-+-+-+-+-+-+- //
//        Include        //
// -+-+-+-+-+-+-+-+-+-+- //

#include "render.hpp"
