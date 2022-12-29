#pragma once
#include <algorithm>
#include <array>
#include <complex>
#include <utility>
#include <cmath>
#include <initializer_list>

#ifdef __CUDACC__
#define GLOBAL_FUNC_PREFIX __host__ __device__
#else
#define GLOBAL_FUNC_PREFIX
#endif

namespace util {

// -+-+-+-+-+-+-+-+-+-+- //
//     Color Utility     //
// -+-+-+-+-+-+-+-+-+-+- //

GLOBAL_FUNC_PREFIX float min_f(float x, float y){
	return x < y ? x : y;
}

GLOBAL_FUNC_PREFIX float max_f(float x, float y){
	return x < y ? y : x;
}

GLOBAL_FUNC_PREFIX inline float saturate(float x){
	return max_f(0.f, min_f(1.f, x));
}

GLOBAL_FUNC_PREFIX  inline float lerp(float a, float b, float mix){
	return (b-a) * saturate(mix) + a;
}

GLOBAL_FUNC_PREFIX inline float lerp_multi(std::initializer_list<float> fl, float mix){
	assert(2 <= fl.size());
	float section_mix = saturate(mix) * (fl.size()-1);
	int section = section_mix;
	section_mix -= section;

	if(section < fl.size() - 1){
		return lerp(*(fl.begin() + section), *(fl.begin() + section+1), section_mix);
	}else{
		return *(fl.begin() + (fl.size()-1));
	}
}

GLOBAL_FUNC_PREFIX inline float dot(const float a[2], const float b[2]){
	return a[0]*b[0] + a[1]*b[1];
}

GLOBAL_FUNC_PREFIX inline float cross(const float a[2], const float b[2]){
	return a[0]*b[1] - a[1]*b[0];
}

GLOBAL_FUNC_PREFIX inline float byte2float(unsigned char x){
	return (0.5 + float(x)) / 255;
}

GLOBAL_FUNC_PREFIX inline unsigned char float2byte(float x){
	int tmp = x * 255;
	return static_cast<unsigned char>(max_f(0, min_f(255, tmp)));
}

GLOBAL_FUNC_PREFIX inline RGBA invert_color(const RGBA& col){
	using uc = unsigned char;
	return RGBA{ uc(255-col.r), uc(255-col.g), uc(255-col.b), col.a };
}


// -+-+-+-+-+-+-+-+-+-+- //
//         Blend         //
// -+-+-+-+-+-+-+-+-+-+- //

template<class T>
GLOBAL_FUNC_PREFIX inline RGBA blend_internal(const RGBA& dst, const RGBA& src, float Fd, float Fs, T B){
	// https://ja.wikipedia.org/wiki/%E3%82%A2%E3%83%AB%E3%83%95%E3%82%A1%E3%83%81%E3%83%A3%E3%83%B3%E3%83%8D%E3%83%AB
	// https://qiita.com/kerupani129/items/4bf75d9f44a5b926df58#0-%E3%81%BE%E3%81%A8%E3%82%81
	// https://ja.wikipedia.org/wiki/%E3%83%96%E3%83%AC%E3%83%B3%E3%83%89%E3%83%A2%E3%83%BC%E3%83%89
	float Ad = dst.a/255.f, As = src.a/255.f;
	const float Cd[3]{ byte2float(dst.r), byte2float(dst.g), byte2float(dst.b) };
	const float Cs[3]{ byte2float(src.r), byte2float(src.g), byte2float(src.b) };

	const float alpha = saturate(Ad*Fd + As*Fs);
	if(alpha * 255 < 1)
		return {0, 0, 0, 0};

	const RGBA_f b = B(Cd, Cs);

	const float C_tmp[3]{
		Ad*b.r + (1-Ad)*Cs[0],
		Ad*b.g + (1-Ad)*Cs[1],
		Ad*b.b + (1-Ad)*Cs[2]
	};
	const float C[3]{
		(Ad*Fd*Cd[0] + As*Fs*C_tmp[0]) / alpha,
		(Ad*Fd*Cd[1] + As*Fs*C_tmp[1]) / alpha,
		(Ad*Fd*Cd[2] + As*Fs*C_tmp[2]) / alpha
	};
	return {
		float2byte(C[0]),
		float2byte(C[1]),
		float2byte(C[2]),
		float2byte(alpha)
	};
}

GLOBAL_FUNC_PREFIX inline RGBA_f internal_source_normal(const float dst[3], const float src[3]){
	return { src[0], src[1], src[2], 0 }; 
}

GLOBAL_FUNC_PREFIX inline RGBA_f internal_source_multiply(const float dst[3], const float src[3]){
	return { dst[0]*src[0], dst[1]*src[1], dst[2]*src[2], 0 }; 
}

GLOBAL_FUNC_PREFIX inline RGBA_f internal_source_add(const float dst[3], const float src[3]){
	return { saturate(dst[0]+src[0]), saturate(dst[1]+src[1]), saturate(dst[2]+src[2]), 0 }; 
}


// -+-+-+-+-+-+-+-+-+-+- //
//      Blend Modes      //
// -+-+-+-+-+-+-+-+-+-+- //

// 通常
GLOBAL_FUNC_PREFIX inline RGBA blend_normal(const RGBA& dst, const RGBA& src){
	return blend_internal(dst, src, 1-src.a/255.f, 1, internal_source_normal);
}

// 乗算
GLOBAL_FUNC_PREFIX inline RGBA blend_multiply(const RGBA& dst, const RGBA& src){
	return blend_internal(dst, src, 1-src.a/255.f, 1, internal_source_multiply);
}

// スクリーン
GLOBAL_FUNC_PREFIX inline RGBA blend_screen(const RGBA& dst, const RGBA& src){
	return invert_color(blend_internal(invert_color(dst), invert_color(src), 1-src.a/255.f, 1, internal_source_multiply));
}

// 加算
GLOBAL_FUNC_PREFIX inline RGBA blend_add(const RGBA& dst, const RGBA& src){
	return blend_internal(dst, src, 1-src.a/255.f, 1, internal_source_add);
}

// 加算発光
GLOBAL_FUNC_PREFIX inline RGBA blend_plus_normal(const RGBA& dst, const RGBA& src){
	return blend_internal(dst, src, 1, 1, internal_source_normal);
}

// 加算発光の強い版？ めっちゃ光りそう。
GLOBAL_FUNC_PREFIX inline RGBA blend_plus_add(const RGBA& dst, const RGBA& src){
	return blend_internal(dst, src, 1, 1, internal_source_add);
}

// xor
GLOBAL_FUNC_PREFIX inline RGBA blend_xor(const RGBA& dst, const RGBA& src){
	return blend_internal(dst, src, 1-src.a/255.f, 1-dst.a/255.f, internal_source_normal);
}

}
