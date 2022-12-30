#include <algorithm>
#include <array>
#include <complex>
#include <cmath>
#include "protocol.hpp"
#include "blend.hpp"

namespace renderer_cpu {

// -+-+-+-+-+-+-+-+-+-+-+- //
//     CPU / Initialize    //
// -+-+-+-+-+-+-+-+-+-+-+- //

void init(Status& status){
	/**
	以下の値を適宜書き換えられる。
	const float status.fps;
	const int   status.duration;
	const int   status.height;
	const int   status.width;
	*/

	status.duration = 1;
}


// -+-+-+-+-+-+-+-+-+-+-+- //
//     CPU / Rendering     //
// -+-+-+-+-+-+-+-+-+-+-+- //

// CPUでの描画処理
// 1フレーム分の描画を行う
inline void render(cv::Mat& img, const Status status){
	using util::saturate;
	using util::dot;
	using util::lerp;
	using util::lerp_multi;
	[[maybe_unused]] const int   frame    = status.frame;
	[[maybe_unused]] const float time     = status.time;
	[[maybe_unused]] const float fps      = status.fps;
	[[maybe_unused]] const float duration = status.duration;
	[[maybe_unused]] const int   height   = status.height;
	[[maybe_unused]] const int   width    = status.width;
	// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- //

	for(int y=0; y<height; ++y){
		for(int x=0; x<width; ++x){  // 各ピクセルに対して処理を行う
			const cv::Vec4b col_vec4 = img.at<cv::Vec4b>(y, x);  // 今見ているピクセルの色への参照
			RGBA col = {col_vec4[0], col_vec4[1], col_vec4[2], col_vec4[3]};

			// ここに処理を書く
			col.r = x % 256;
			col.g = y % 256;
			col.b = frame * 16 % 256;
			// ここまで

			img.at<cv::Vec4b>(y, x) = cv::Vec4b(col.b, col.g, col.r, col.a);
		}
	}
}

}


#ifdef __CUDACC__
namespace renderer_gpu {

// -+-+-+-+-+-+-+-+-+-+-+- //
//     GPU / Initialize    //
// -+-+-+-+-+-+-+-+-+-+-+- //

void init(Status& status){
	/**
	以下の値を適宜書き換えられる。
	const float status.fps;
	const int   status.duration;
	const int   status.height;
	const int   status.width;
	*/

	status.duration = 1;
}


// -+-+-+-+-+-+-+-+-+-+-+- //
//     GPU / Rendering     //
// -+-+-+-+-+-+-+-+-+-+-+- //

// GPUでの描画処理
// 1ピクセル分の描画を行う
__device__ RGBA render(unsigned char* img, const Status status, int x, int y){
	using util::saturate;
	using util::dot;
	using util::lerp;
	using util::lerp_multi;
	[[maybe_unused]] const int   frame    = status.frame;
	[[maybe_unused]] const float time     = status.time;
	[[maybe_unused]] const float fps      = status.fps;
	[[maybe_unused]] const float duration = status.duration;
	[[maybe_unused]] const int   height   = status.height;
	[[maybe_unused]] const int   width    = status.width;
	// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- //

	RGBA col{};  // 最終的に格納する色

	// ここに処理を書く
	col.r = x % 256;
	col.g = y % 256;
	col.b = frame * 16 % 256;
	// ここまで

	return col;
}

}
#endif
