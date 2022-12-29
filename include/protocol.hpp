#pragma once
#include <array>
#include <opencv2/opencv.hpp>

struct Status {
	int frame;
	float fps;
	float time;
	float duration;
	int height;
	int width;
};

struct RGBA{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
};

struct RGBA_f{
	float r;
	float g;
	float b;
	float a;
};

namespace renderer_cpu {
void init(Status& status);
void render(cv::Mat& img, const Status status);
}

namespace renderer_gpu {
void init(Status& status);
RGBA render(unsigned char* img, const Status status, int x, int y);
}
