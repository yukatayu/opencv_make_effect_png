#include <iostream>
#include <cstdio>
#include <string>
#include <iomanip>
#include <sstream>
#include <opencv2/opencv.hpp>

std::string zero_ume(int i, int width = 6){
	std::ostringstream ss;
	ss << std::setfill('0') << std::right << std::setw(width) << i;
	return ss.str();
}

int main(){
	// メモ： 読み込みは
	// cv::Mat transimg = cv::imread("transparent.png",cv::IMREAD_UNCHANGED);

	for(int frame = 0; frame < 30; ++frame){
		cv::Mat img(cv::Size(1920, 1080), CV_MAKE_TYPE(CV_8U, 4));
		
		for(int y=0; y<img.size().height; ++y){
			for(int x=0; x<img.size().width; ++x){
				cv::Vec4b col(x%256, (x+y)%256, 0, (y+frame)%256);
				img.at<cv::Vec4b>(y,x) = col;
			}
		}

		std::ostringstream file_name;
		file_name << "png/out_" << zero_ume(frame) << ".png";
		cv::imwrite(file_name.str(), img);
	}
}
