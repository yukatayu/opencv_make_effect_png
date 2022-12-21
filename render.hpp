#include <algorithm>

void init(Status& status){
	/**
	以下の値を適宜書き換えられる。
	主に duration の設定用。

	const float status.fps;
	const int   status.duration;
	const int   status.height;
	const int   status.width;
	*/
}

void render(cv::Mat& img, const Status status){
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
