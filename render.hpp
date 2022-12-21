#include <algorithm>
#include <array>
#include <optional>
#include <complex>

// -+-+-+-+-+-+-+-+-+-+- //
//       Initialize      //
// -+-+-+-+-+-+-+-+-+-+- //

void init(Status& status){
	/**
	以下の値を適宜書き換えられる。
	const float status.fps;
	const int   status.duration;
	const int   status.height;
	const int   status.width;
	*/

	status.duration = 2;
}


// -+-+-+-+-+-+-+-+-+-+- //
//         Blend         //
// -+-+-+-+-+-+-+-+-+-+- //

inline float saturate(float x){
	return std::max(0.f, std::min(1.f, x));
}

inline float byte2float(unsigned char x){
	return (0.5 + float(x)) / 255;
}

inline unsigned char float2byte(float x){
	int tmp = x * 255;
	return static_cast<unsigned char>(std::max(0, std::min(255, tmp)));
}

template<class T>
inline void blend_internal(cv::Vec4b& dst, const cv::Vec4b src, float Fd, float Fs, T B){
	// https://ja.wikipedia.org/wiki/%E3%82%A2%E3%83%AB%E3%83%95%E3%82%A1%E3%83%81%E3%83%A3%E3%83%B3%E3%83%8D%E3%83%AB
	// https://qiita.com/kerupani129/items/4bf75d9f44a5b926df58#0-%E3%81%BE%E3%81%A8%E3%82%81
	float Ad = dst[3]/255.f, As = src[3]/255.f;
	const std::array<float,3> Cd{ byte2float(dst[0])*Ad, byte2float(dst[1]), byte2float(dst[2]) };
	const std::array<float,3> Cs{ byte2float(src[0]), byte2float(src[1]), byte2float(src[2]) };

	const float alpha = saturate(Ad*Fd + As*Fs);
	if(alpha * 255 < 1){
		dst = 0;
		return ;
	}
	std::array<float,3> b = B(Cd, Cs);
	const std::array<float,3> C_tmp{
		Ad*b[0] + (1-Ad)*src[0],
		Ad*b[1] + (1-Ad)*src[1],
		Ad*b[2] + (1-Ad)*src[2]
	};
	const std::array<float, 3> C{
		saturate((Ad*Fd*Cd[0] + As*Fs*C_tmp[0]) / alpha),
		saturate((Ad*Fd*Cd[1] + As*Fs*C_tmp[1]) / alpha),
		saturate((Ad*Fd*Cd[2] + As*Fs*C_tmp[2]) / alpha)
	};
	dst = cv::Vec4b{
		float2byte(C[0]),
		float2byte(C[1]),
		float2byte(C[2]),
		float2byte(alpha)
	};
}

inline std::array<float,3> internal_source_normal(const std::array<float,3>& dst, const std::array<float,3>& src){
	return std::array<float,3> { saturate(src[0]), saturate(src[1]), saturate(src[2]) }; 
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
inline void blend_normal(cv::Vec4b& dst, const cv::Vec4b src){
	blend_internal(dst, src, 1-src[3]/255.f, 1,internal_source_normal);
}

// 乗算
inline void blend_multiply(cv::Vec4b& dst, const cv::Vec4b src){
	blend_internal(dst, src, 1-src[3]/255.f, 1,internal_source_multiply);
}

// 加算
inline void blend_add(cv::Vec4b& dst, const cv::Vec4b src){
	blend_internal(dst, src, 1-src[3]/255.f, 1,internal_source_add);
}

// 加算発光
inline void blend_plus_normal(cv::Vec4b& dst, const cv::Vec4b src){
	blend_internal(dst, src, 1, 1, internal_source_normal);
}

// 加算発光の強い版？ めっちゃ光りそう。
inline void blend_plus_add(cv::Vec4b& dst, const cv::Vec4b src){
	blend_internal(dst, src, 1, 1, internal_source_add);
}

// xor
inline void blend_xor(cv::Vec4b& dst, const cv::Vec4b src){
	blend_internal(dst, src, 1-src[3]/255.f, 1-dst[3]/255.f, internal_source_normal);
}


// -+-+-+-+-+-+-+-+-+-+- //
//       Rendering       //
// -+-+-+-+-+-+-+-+-+-+- //

// 四角形の描画 (rot = 0～1)
// 返り値： x,yの順， [-1,1] の範囲 or nullopt
inline std::optional<std::array<float,2>> rectangle(float dx, float dy, float width, float height, float rot = 0){
	// 回転する： (x+yi) * e^{2πi * rot}
	auto xy = std::complex<double>(dx, dy) * std::exp(std::complex<double>(0, rot*2*M_PI));
	std::array<float,2> res{ float(xy.real() / width), float(xy.imag() / height) };
	if(std::abs(res[0]) <= 1 && std::abs(res[1]) <= 1)
		return res;
	else
		return std::nullopt;

}

// 円の描画
// 返り値： [0,1] の範囲 or nullopt
inline std::optional<float> circle(float dx, float dy, float radius){
	float res = dx*dx + dy*dy / radius*radius;
	if(res <= 1)
		return res;
	else
		std::nullopt;
}

inline void render(cv::Mat& img, const Status status){
	const int   frame    = status.frame;
	const float time     = status.time;
	const float fps      = status.fps;
	const int   duration = status.duration;
	const int   height   = status.height;
	const int   width    = status.width;

	// ここから下がメインの色の指定処理
	for(int y=0; y<img.size().height; ++y){
		for(int x=0; x<img.size().width; ++x){
			cv::Vec4b col(
				x%256,                    // r
				(x+y)%256,                // g
				int(status.time*128)%256, // b
				int(y+time*256)%256       // a
			);
			img.at<cv::Vec4b>(y,x) = col;
		}
	}

}
