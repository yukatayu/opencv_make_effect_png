#include <algorithm>
#include <array>
#include <optional>
#include <complex>

namespace renderer {

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
// 返り値： x,yの順， [-1,1] の範囲 or nullopt
inline std::optional<std::array<float,2>> rectangle(float dx, float dy, float width, float height, float rot = 0, bool tile = false){
	// 回転する： (x+yi) * e^{2πi * rot}
	auto xy = std::complex<double>(dx, dy) * std::exp(std::complex<double>(0, rot*2*M_PI));
	
	std::array<float,2> res{ float(xy.real() / width), float(xy.imag() / height) };
	if(-1 <= res[0] && res[0] < 1 && -1 <= res[1] && res[1] < 1)  // 1pxだけ重なるのを防止
		return res;
	if(!tile && (-1 <= res[0] && res[0] <= 1 && -1 <= res[1] && res[1] <= 1))
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

// メインの描画処理
inline void render(cv::Mat& img, const Status status){
	const int   frame    = status.frame;
	const float time     = status.time;
	const float fps      = status.fps;
	const float duration = status.duration;
	const int   height   = status.height;
	const int   width    = status.width;
	// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- //

	// 正方形を並べるやつのパラメータたち
	const int vert_cnt = 9;  // 縦に並べる個数
	const float vert_unit = height / vert_cnt;  // 正方形の大きさ
	const int hori_cnt = std::ceil(width / vert_unit);
	const float time_in = 0.4;  // フェードインの時間
	const float time_delta = 0.06;  // タイミングのズレ
	const float time_mid_stop = 0.2;  // 塗りつぶし状態で一旦止まる時間
	const float time_start_rev = (hori_cnt + vert_cnt - 2) * time_delta + time_in + time_mid_stop;

	for(int y=0; y<img.size().height; ++y){
		for(int x=0; x<img.size().width; ++x){  // 各ピクセルに対して処理を行う
			cv::Vec4b& col = img.at<cv::Vec4b>(y, x);  // 今見ているピクセルの色への参照

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
					if(maybe_square){
						auto[cx, cy] = *maybe_square;  // 四角形の中での今の位置の座標 [-1,1]
						
						// 透明度を良い感じにする
						float opacity = pow(std::max(std::abs(cx), std::abs(cy)), 0.4);  // 端に行くにつれて1に近づく(minなので四角っぽくなるはず)
						opacity = 1 - opacity * saturate(2*(1-anim_time));  // 出現しきった時には不透明
						opacity *= 1 - (1-anim_time);  // だんだんと不透明になりながら出現
						opacity = saturate(opacity);

						// グラデーションの処理
						std::array<float,2> vec_diagonal{ float(width), float(height) }, pos{ float(x), float(y) };
						float gradation = dot(vec_diagonal, pos) / dot(vec_diagonal, vec_diagonal);

						// 色はここの数値： https://www.colordic.org/colorsample/7049
						col = blend_screen(col, cv::Vec4b(
							lerp(255, 162, gradation),
							lerp(147, 143, gradation),
							lerp(201, 255, gradation),
							opacity * 255));
					}
				}
			}
		}
	}
}

}
