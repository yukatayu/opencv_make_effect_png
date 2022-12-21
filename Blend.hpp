#include <algorithm>
#include <array>
#include <complex>
#include <utility>
#include <cmath>

namespace renderer {
// -+-+-+-+-+-+-+-+-+-+- //
//         Blend         //
// -+-+-+-+-+-+-+-+-+-+- //

inline float saturate(float x){
	return std::max(0.f, std::min(1.f, x));
}

inline float lerp(float a, float b, float mix){
	return (b-a) * saturate(mix) + a;
}

inline float dot(std::array<float,2> a, std::array<float,2> b){
	return a[0]*b[0] + a[1]*b[1];
}

inline float cross(std::array<float,2> a, std::array<float,2> b){
	return a[0]*b[1] - a[1]*b[0];
}

inline float byte2float(unsigned char x){
	return (0.5 + float(x)) / 255;
}

inline unsigned char float2byte(float x){
	int tmp = x * 255;
	return static_cast<unsigned char>(std::max(0, std::min(255, tmp)));
}

inline cv::Vec4b invert_color(const cv::Vec4b& col){
	return cv::Vec4b(255-col[0], 255-col[1], 255-col[2], col[3]);
}

template<class T>
inline cv::Vec4b blend_internal(const cv::Vec4b& dst, const cv::Vec4b& src, float Fd, float Fs, T B){
	// https://ja.wikipedia.org/wiki/%E3%82%A2%E3%83%AB%E3%83%95%E3%82%A1%E3%83%81%E3%83%A3%E3%83%B3%E3%83%8D%E3%83%AB
	// https://qiita.com/kerupani129/items/4bf75d9f44a5b926df58#0-%E3%81%BE%E3%81%A8%E3%82%81
	// https://ja.wikipedia.org/wiki/%E3%83%96%E3%83%AC%E3%83%B3%E3%83%89%E3%83%A2%E3%83%BC%E3%83%89
	float Ad = dst[3]/255.f, As = src[3]/255.f;
	const std::array<float,3> Cd{ byte2float(dst[0]), byte2float(dst[1]), byte2float(dst[2]) };
	const std::array<float,3> Cs{ byte2float(src[0]), byte2float(src[1]), byte2float(src[2]) };

	const float alpha = saturate(Ad*Fd + As*Fs);
	if(alpha * 255 < 1)
		return 0;

	std::array<float,3> b = B(Cd, Cs);

	const std::array<float,3> C_tmp{
		Ad*b[0] + (1-Ad)*Cs[0],
		Ad*b[1] + (1-Ad)*Cs[1],
		Ad*b[2] + (1-Ad)*Cs[2]
	};
	const std::array<float, 3> C{
		(Ad*Fd*Cd[0] + As*Fs*C_tmp[0]) / alpha,
		(Ad*Fd*Cd[1] + As*Fs*C_tmp[1]) / alpha,
		(Ad*Fd*Cd[2] + As*Fs*C_tmp[2]) / alpha
	};
	return cv::Vec4b{
		float2byte(C[0]),
		float2byte(C[1]),
		float2byte(C[2]),
		float2byte(alpha)
	};
}

inline std::array<float,3> internal_source_normal(const std::array<float,3>& dst, const std::array<float,3>& src){
	return std::array<float,3> { src[0], src[1], src[2] }; 
}

inline std::array<float,3> internal_source_multiply(const std::array<float,3>& dst, const std::array<float,3>& src){
	return std::array<float,3> { dst[0]*src[0], dst[1]*src[1], dst[2]*src[2] }; 
}

inline std::array<float,3> internal_source_add(const std::array<float,3>& dst, const std::array<float,3>& src){
	return std::array<float,3> { saturate(dst[0]+src[0]), saturate(dst[1]+src[1]), saturate(dst[2]+src[2]) }; 
}


// -+-+-+-+-+-+-+-+-+-+- //
//      Blend Modes      //
// -+-+-+-+-+-+-+-+-+-+- //

// 通常
inline cv::Vec4b blend_normal(const cv::Vec4b& dst, const cv::Vec4b& src){
	return blend_internal(dst, src, 1-src[3]/255.f, 1, internal_source_normal);
}

// 乗算
inline cv::Vec4b blend_multiply(const cv::Vec4b& dst, const cv::Vec4b& src){
	return blend_internal(dst, src, 1-src[3]/255.f, 1, internal_source_multiply);
}

// スクリーン
inline cv::Vec4b blend_screen(const cv::Vec4b& dst, const cv::Vec4b& src){
	return invert_color(blend_internal(invert_color(dst), invert_color(src), 1-src[3]/255.f, 1, internal_source_multiply));
}

// 加算
inline cv::Vec4b blend_add(const cv::Vec4b& dst, const cv::Vec4b& src){
	return blend_internal(dst, src, 1-src[3]/255.f, 1, internal_source_add);
}

// 加算発光
inline cv::Vec4b blend_plus_normal(const cv::Vec4b& dst, const cv::Vec4b& src){
	return blend_internal(dst, src, 1, 1, internal_source_normal);
}

// 加算発光の強い版？ めっちゃ光りそう。
inline cv::Vec4b blend_plus_add(const cv::Vec4b& dst, const cv::Vec4b& src){
	return blend_internal(dst, src, 1, 1, internal_source_add);
}

// xor
inline cv::Vec4b blend_xor(const cv::Vec4b& dst, const cv::Vec4b& src){
	return blend_internal(dst, src, 1-src[3]/255.f, 1-dst[3]/255.f, internal_source_normal);
}

}
