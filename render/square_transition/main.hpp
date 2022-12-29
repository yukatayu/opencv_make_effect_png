#include <algorithm>
#include <array>
#include <complex>
#include <cmath>
#include "protocol.hpp"
#include "blend.hpp"

namespace renderer_cpu {

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

	status.duration = 1;
	// return ;  // デバッグ用
	
	const int vert_cnt = 9;  // 縦に並べる個数
	const float vert_unit = status.height / vert_cnt;  // 正方形の大きさ
	const int hori_cnt = std::ceil(status.width / vert_unit);
	const float time_in = 0.4;  // フェードインの時間
	const float time_delta = 0.06;  // タイミングのズレ
	const float time_mid_stop = 0.2;  // 塗りつぶし状態で一旦止まる時間
	status.duration = ((hori_cnt + vert_cnt - 2) * time_delta + time_in) * 2 + time_mid_stop;
}


// -+-+-+-+-+-+-+-+-+-+- //
//       Rendering       //
// -+-+-+-+-+-+-+-+-+-+- //

// 四角形の描画 (rot = 0～1)
// 返り値： 範囲内かどうか， x,y([-1,1] の範囲)
inline std::pair<bool, std::array<float,2>> rectangle(float dx, float dy, float width, float height, float rot = 0, bool tile = false){
	// 回転する： (x+yi) * e^{2πi * rot}
	auto xy = std::complex<double>(dx, dy) * std::exp(std::complex<double>(0, rot*2*M_PI));
	
	std::array<float,2> res{ float(xy.real() / width), float(xy.imag() / height) };
	if(-1 <= res[0] && res[0] < 1 && -1 <= res[1] && res[1] < 1)  // 1pxだけ重なるのを防止
		return {true, res};
	if(!tile && (-1 <= res[0] && res[0] <= 1 && -1 <= res[1] && res[1] <= 1))
		return {true, res};
	else
		return {};
}

// 円の描画
// 返り値： 範囲内かどうか， r([0,1] の範囲)
inline std::pair<bool, float> circle(float dx, float dy, float radius){
	float res = dx*dx + dy*dy / radius*radius;
	if(res <= 1)
		return {true, res};
	else
		return {};
}

// メインの描画処理
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

	// 正方形を並べるやつのパラメータたち
	const int vert_cnt = 9;  // 縦に並べる個数
	const float vert_unit = height / vert_cnt;  // 正方形の大きさ
	const int hori_cnt = std::ceil(width / vert_unit);
	const float time_in = 0.4;  // フェードインの時間
	const float time_delta = 0.06;  // タイミングのズレ
	const float time_mid_stop = 0.2;  // 塗りつぶし状態で一旦止まる時間
	const float time_start_rev = (hori_cnt + vert_cnt - 2) * time_delta + time_in + time_mid_stop;

	for(int y=0; y<height; ++y){
		for(int x=0; x<width; ++x){  // 各ピクセルに対して処理を行う
			const cv::Vec4b col_vec4 = img.at<cv::Vec4b>(y, x);  // 今見ているピクセルの色への参照
			RGBA col = {col_vec4[0], col_vec4[1], col_vec4[2], col_vec4[3]};

			for(int sy = 0; sy < vert_cnt; ++sy){
				for(int sx = 0; sx < hori_cnt; ++sx){  //縦横に正方形を並べる
					const int sid = sy + sx;
					const float anim_time_phase[2] {
						saturate((time - time_delta*sid) / time_in),
						saturate((time - time_start_rev - time_delta*sid) / time_in)
					};
					const float anim_time =  // 0～1の間でアニメーションする
						std::min(anim_time_phase[0], 1 - anim_time_phase[1]);
					// const float anim_time_for_rot = 0 < anim_time_phase[1] ? anim_time_phase[1] : anim_time_phase[0];
					if(anim_time == 0)
						continue;  // この正方形は，まだ出現していない
					
					// 四角形の大きさと回転を良い感じに設定
					const float scale = 1 - (1-anim_time) * 0.4;
					const float rot = (1-anim_time) * 0.2;

					// タイルの内側かどうかの判定
					auto maybe_square =
						rectangle(
							x - (sx+0.5)*vert_unit,  // 正方形の中心と今のx座標の差
							y - (sy+0.5)*vert_unit,
							vert_unit/2 * scale,  // 一辺
							vert_unit/2 * scale,
							rot,
							true  // タイルにしたときに1pxだけ重なるのを防止する
						);

					// 内側だった場合は色を描画
					if(maybe_square.first){
						// auto [cx, cy] = maybe_square.second;  // 四角形の中での今の位置の座標 [-1,1]
						auto cx = maybe_square.second[0];
						auto cy = maybe_square.second[1];
						
						// 透明度を良い感じにする
						// float opacity = std::pow(std::max(std::abs(cx), std::abs(cy)), 0.4);  // 端に行くにつれて1に近づく(minなので四角っぽくなるはず)
						float opacity = std::pow(cx*cx+cy*cy, 0.3);  // 端に行くにつれて1に近づく(丸っぽくする)
						opacity = 1 - opacity * saturate(2*(1-anim_time));  // 出現しきった時には不透明
						opacity *= 1 - (1-anim_time);  // だんだんと不透明になりながら出現
						opacity = saturate(opacity);

						// グラデーションの処理
						const float vec_diagonal[2]{ float(width), float(height) }, pos[2]{ float(x), float(y) };
						float gradation = dot(vec_diagonal, pos) / dot(vec_diagonal, vec_diagonal);

						// 注意： 色はBGRAの順に格納される。
						RGBA target_col {
							lerp_multi({215,215,204,125, 90, 87, 53, 39}, 1-gradation),
							lerp_multi({220,220,201, 98, 35, 19, 22, 26}, 1-gradation),
							lerp_multi({219,221,226,231,184,128, 61, 28}, 1-gradation),
							(unsigned char)(opacity * 255)
						};
						col = util::blend_screen(col, target_col);
					}
				}
			}
			img.at<cv::Vec4b>(y, x) = cv::Vec4b(col.b, col.g, col.r, col.a);
		}
	}
}

}


#ifdef __CUDACC__
namespace renderer_gpu {

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

	status.duration = 1;
	// return ;  // デバッグ用
	
	const int vert_cnt = 9;  // 縦に並べる個数
	const float vert_unit = status.height / vert_cnt;  // 正方形の大きさ
	const int hori_cnt = std::ceil(status.width / vert_unit);
	const float time_in = 0.4;  // フェードインの時間
	const float time_delta = 0.06;  // タイミングのズレ
	const float time_mid_stop = 0.2;  // 塗りつぶし状態で一旦止まる時間
	status.duration = ((hori_cnt + vert_cnt - 2) * time_delta + time_in) * 2 + time_mid_stop;
}


// -+-+-+-+-+-+-+-+-+-+- //
//       Rendering       //
// -+-+-+-+-+-+-+-+-+-+- //

// 四角形の描画 (rot = 0～1)
// 返り値： 範囲内かどうか， x,y([-1,1] の範囲)
__device__ bool rectangle(float res[2], float dx, float dy, float width, float height, float rot = 0, bool tile = false){
	// 回転する： (x+yi) * e^{2πi * rot}
	// auto xy = std::complex<double>(dx, dy) * std::exp(std::complex<double>(0, rot*2*M_PI));
	// std::array<float,2> res{ float(xy.real() / width), float(xy.imag() / height) };

	float r[2]{ float(std::cos(rot*2*M_PI)), float(std::sin(rot*2*M_PI))};
	float xy[2]{ r[0]*dx - r[1]*dy, r[0]*dy + r[1]*dx };
	res[0] = xy[0] / width;
	res[1] = xy[1] / height;

	if(-1 <= res[0] && res[0] < 1 && -1 <= res[1] && res[1] < 1)  // 1pxだけ重なるのを防止
		return true;
	
	if(!tile && (-1 <= res[0] && res[0] <= 1 && -1 <= res[1] && res[1] <= 1))
		return true;
	else
		return false;
}

// メインの描画処理
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

	// 正方形を並べるやつのパラメータたち
	const int vert_cnt = 9;  // 縦に並べる個数
	const float vert_unit = height / vert_cnt;  // 正方形の大きさ
	const int hori_cnt = std::ceil(width / vert_unit);
	const float time_in = 0.4;  // フェードインの時間
	const float time_delta = 0.06;  // タイミングのズレ
	const float time_mid_stop = 0.2;  // 塗りつぶし状態で一旦止まる時間
	const float time_start_rev = (hori_cnt + vert_cnt - 2) * time_delta + time_in + time_mid_stop;

	RGBA col{};  // 最終的に格納する色

	for(int sy = 0; sy < vert_cnt; ++sy){
		for(int sx = 0; sx < hori_cnt; ++sx){  //縦横に正方形を並べる
			const int sid = sy + sx;
			const float anim_time_phase[2] {
				saturate((time - time_delta*sid) / time_in),
				saturate((time - time_start_rev - time_delta*sid) / time_in)
			};
			const float anim_time =  // 0～1の間でアニメーションする
				std::min(anim_time_phase[0], 1 - anim_time_phase[1]);
			// const float anim_time_for_rot = 0 < anim_time_phase[1] ? anim_time_phase[1] : anim_time_phase[0];
			if(anim_time == 0)
				continue;  // この正方形は，まだ出現していない
			
			// 四角形の大きさと回転を良い感じに設定
			const float scale = 1 - (1-anim_time) * 0.4;
			const float rot = (1-anim_time) * 0.2;

			// タイルの内側かどうかの判定
			float square[2]{};
			bool ok =
				rectangle(
					square,
					x - (sx+0.5)*vert_unit,  // 正方形の中心と今のx座標の差
					y - (sy+0.5)*vert_unit,
					vert_unit/2 * scale,  // 一辺
					vert_unit/2 * scale,
					rot,
					true  // タイルにしたときに1pxだけ重なるのを防止する
				);

			// 内側だった場合は色を描画
			if(ok){
				// 四角形の中での今の位置の座標 [-1,1]
				auto cx = square[0];
				auto cy = square[1];
				
				// 透明度を良い感じにする
				// float opacity = std::pow(std::max(std::abs(cx), std::abs(cy)), 0.4);  // 端に行くにつれて1に近づく(minなので四角っぽくなるはず)
				float opacity = std::pow(cx*cx+cy*cy, 0.3);  // 端に行くにつれて1に近づく(丸っぽくする)
				opacity = 1 - opacity * saturate(2*(1-anim_time));  // 出現しきった時には不透明
				opacity *= 1 - (1-anim_time);  // だんだんと不透明になりながら出現
				opacity = saturate(opacity);

				// グラデーションの処理
				const float vec_diagonal[2]{ float(width), float(height) }, pos[2]{ float(x), float(y) };
				float gradation = dot(vec_diagonal, pos) / dot(vec_diagonal, vec_diagonal);

				// 注意： 色はBGRAの順に格納される。
				RGBA target_col {
					lerp_multi({215,215,204,125, 90, 87, 53, 39}, 1-gradation),
					lerp_multi({220,220,201, 98, 35, 19, 22, 26}, 1-gradation),
					lerp_multi({219,221,226,231,184,128, 61, 28}, 1-gradation),
					(unsigned char)(opacity * 255)
				};
				col = util::blend_screen(col, target_col);
			}
		}
	}
	return col;
}

}
#endif
