#include "WdgtEdvsVisual.h"
#include <QtGui/QLabel>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <time.h>


constexpr unsigned int RetinaSize = 128;
constexpr int cDecay = 24;
constexpr int cDisplaySize = 128;
constexpr int cUpdateInterval = 10;
constexpr int cDisplayInterval = 20;

constexpr unsigned int HoughSizeDist = 256;
constexpr unsigned int HoughSizeAlpha = 256;

// blue/yellow color scheme
// const QRgb cColorMid = qRgb(0, 0, 0);
// const QRgb cColorOn = qRgb(255, 255, 0);
// const QRgb cColorOff = qRgb(0, 0, 255);

// black/white color scheme
const QRgb cColorMid = qRgb(128, 128, 128);
const QRgb cColorOn = qRgb(255, 255, 255);
const QRgb cColorOff = qRgb(0, 0, 0);



namespace impl
{
	int map_d2i(float d) {
		constexpr float DMAX = 1.4142f*static_cast<float>(RetinaSize);
		return std::floor(HoughSizeDist*(0.5f+d/(2.0f*DMAX)));
	}
	float map_i2a(int i) {
		return static_cast<float>(i) / static_cast<float>(HoughSizeAlpha) * 3.1415f;
	}
}

struct CrossModel
{
	static constexpr int S = RetinaSize;
	static constexpr float PI = 3.141592654f;
	static constexpr int NA = 32;

	int r;

	Eigen::MatrixXf ta, tw;
	std::vector<Eigen::MatrixXf> data;
	Eigen::MatrixXf data_w;

	CrossModel(float sigma=15.0f) {
		r = std::max<int>(3, std::ceil(3.0f * sigma)); // TODO 3.0f is only an approximation
		int a = 2*r+1;

		ta = Eigen::MatrixXf::Zero(a,a);
		tw = Eigen::MatrixXf::Zero(a,a);
		float scl1 = 1.0f / std::sqrt(2.0f * PI * sigma);
		float sigma2 = 0.25f * sigma;
		float scl2 = 1.0f / (std::sqrt(5.0f) * std::sqrt(2.0f * PI * sigma2));
		for(int x=-r; x<=+r; x++) {
			for(int y=-r; y<=+r; y++) {					
				float xf = static_cast<float>(x);
				float yf = static_cast<float>(y);
				float angle, weight;
				if(x == 0 && y == 0) {
					angle = 0.0f;
					weight = 0.0f;
				}
				else {
					angle = (std::atan2(yf,xf) + PI) / (2.0f*PI);
					while(angle < 0.0f) angle += 1.0f;
					while(angle >= 1.0f) angle -= 1.0f;
					angle *= static_cast<float>(NA);
					weight = scl1*std::exp(-0.5f*(xf*xf + yf*yf)/(sigma*sigma))
						//- scl2*std::exp(-0.5f*(xf*xf + yf*yf)/(sigma2*sigma2))
						;
				}
				ta(x+r,y+r) = angle;
				tw(x+r,y+r) = std::max(0.0f, weight);
			}
		}

		{
			QImage dbg(a, a, QImage::Format_RGB32);

			float scl = ta.maxCoeff();
			for(int y=0; y<a; y++) {
				for(int x=0; x<a; x++) {
					int v = std::min(255, static_cast<int>(ta(x,y) / scl * 255.0f));
					dbg.setPixel(x, y, qRgb(v,v,v));
				}
			}
			QLabel* label = new QLabel();
			label->setPixmap(QPixmap::fromImage(dbg));
			label->show();
		}

		{
			QImage dbg(a, a, QImage::Format_RGB32);

			float scl = tw.maxCoeff();
			for(int y=0; y<a; y++) {
				for(int x=0; x<a; x++) {
					int v = std::min(255, static_cast<int>(tw(x,y) / scl * 255.0f));
					dbg.setPixel(x, y, qRgb(v,v,v));
				}
			}
			QLabel* label = new QLabel();
			label->setPixmap(QPixmap::fromImage(dbg));
			label->show();
		}

		data = std::vector<Eigen::MatrixXf>(NA, Eigen::MatrixXf::Zero(S,S));
		data_w = Eigen::MatrixXf::Zero(S,S);
	}

	void add_event(int x, int y, int p) {
		if(p == 0) p = -1;
		int x1 = x - r;
		int x2 = x + r;
		int y1 = y - r;
		int y2 = y + r;
		int ax1 = std::max<int>(x1, 0);
		int ax2 = std::min<int>(x2, S-1);
		int ay1 = std::max<int>(y1, 0);
		int ay2 = std::min<int>(y2, S-1);
		int sx = ax2-ax1+1;
		int sy = ay2-ay1+1;
		int bx = (x1 < 0) ? (ax1 - x1) : 0;
		int by = (y1 < 0) ? (ay1 - y1) : 0;
		Eigen::MatrixXf ma = ta.block(bx, by, sx, sy);
		Eigen::MatrixXf mw = tw.block(bx, by, sx, sy);
		for(int y=0; y<sy; ++y) {
			for(int x=0; x<sx; ++x) {
				float w = mw(x,y);
				// float wp = static_cast<float>(p) * w;
				float wp = w;
				float a = ma(x,y);
				int ai = static_cast<int>(a);
				float ap = a - static_cast<float>(ai);
				int gx = ax1 + x;
				int gy = ay1 + y;
				data[(ai + 0) % NA](gx, gy) += wp * (1.0f - ap);
				data[(ai + 1) % NA](gx, gy) += wp * ap;
				data_w(gx, gy) += w;
			}
		}
		//data[0].block(ax1, ay1, sx, sy) += tw.block(bx, by, sx, sy);
	}

	static constexpr float FALLOFF = 0.90f;

	void decay() {
		for(int ai=0; ai<NA; ai++) {
			data[ai] *= FALLOFF;
			// float* src = data[ai].data();
			// for(int i=0; i<S*S; i++) {
			// 	if(*src <= 0.001f) *src = 0.0f;
			// }
		}
		{
			// FIXME w may be wrong when data is clamped!
			data_w *= FALLOFF;
			// float* src = data_w.data();
			// for(int i=0; i<S*S; i++) {
			// 	if(*src <= 0.001f) *src = 0.0f;
			// }
		}
	}

	Eigen::MatrixXf compute_propability()
	{
		Eigen::MatrixXf result = Eigen::MatrixXf::Zero(S, S);
		for(int x=0; x<S; ++x) {
			for(int y=0; y<S; ++y) {
				float v_sum = data_w(x,y);
				if(v_sum <= 0.001f) {
					result(x,y) = 0.0f;
				}
				float p_max = 0.0f;
				// float p_sum = 0.0f;
				for(int ai=0; ai<NA/4; ai++) {
					const float v1 = data[ai](x,y)/v_sum; // 0 degree
					const float v2 = data[ai+NA/4](x,y)/v_sum; // 90 degree
					const float v3 = data[ai+NA/2](x,y)/v_sum; // 180 degree
					const float v4 = data[ai+(3*NA)/4](x,y)/v_sum; // 270 degree
					// float p = 256.0f * std::max(0.0f, -v1*v3) * std::max(0.0f, -v2*v4);
					// float p = 16.0f * (std::max(0.0f, -v1*v3) + std::max(0.0f, -v2*v4));
					float p = 256.0f * std::abs(v1*v2*v3*v4);
					p_max = std::max<float>(p_max, p);
				}
				result(x,y) = v_sum * p_max;
			}
		}
		//std::cout << result.maxCoeff() << std::endl;
		// return result;
		// return 20.0f * result;
		return result / result.maxCoeff();
		// return (result.array() * data_w.array()).matrix();
	}

};

CrossModel* cross_model;



EdvsVisual::EdvsVisual(const Edvs::EventStream& dh, QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	cross_model = new CrossModel();

	image_ = QImage(RetinaSize, RetinaSize, QImage::Format_RGB32);
	img_hough_ = QImage(HoughSizeAlpha, HoughSizeDist, QImage::Format_RGB32);
	img_cross_ = QImage(RetinaSize, RetinaSize, QImage::Format_RGB32);

	hough_ = Eigen::MatrixXf::Constant(HoughSizeAlpha, HoughSizeDist, 0.0f);
	cross_ = Eigen::MatrixXf::Constant(RetinaSize, RetinaSize, 0.0f);

	connect(&timer_update_, SIGNAL(timeout()), this, SLOT(Update()));
	timer_update_.setInterval(cUpdateInterval);
	timer_update_.start();

	connect(&timer_display_, SIGNAL(timeout()), this, SLOT(Display()));
	timer_display_.setInterval(cDisplayInterval);
	timer_display_.start();

	// start capture
	edvs_event_stream_ = dh;
	edvs_event_capture_ = Edvs::EventCapture(edvs_event_stream_,
		boost::bind(&EdvsVisual::OnEvent, this, _1));
}

EdvsVisual::~EdvsVisual()
{
}

void EdvsVisual::OnEvent(const std::vector<Edvs::Event>& newevents)
{
	// just store events
	// protect common vector with a mutex to avoid race conditions
	boost::interprocess::scoped_lock<boost::mutex> lock(events_mtx_);
	events_.insert(events_.end(), newevents.begin(), newevents.end());
	// timespec ts;
	// ts.tv_sec = 0;
	// ts.tv_nsec = 1000;
	// nanosleep(&ts, &ts);

	// print time information
	if(!newevents.empty()) {
		static uint64_t last_time = 0;
		uint64_t current_time = newevents.back().t;
		if(current_time >= last_time + 1000000) {
			std::cout << static_cast<float>(current_time)/1000000.0f << std::endl;
			last_time = current_time;
		}
	}
}	

int DecayComponent(int current, int target, int decay)
{
	if(current-decay >= target) {
		return current - decay;
	}
	if(current+decay <= target) {
		return current + decay;
	}
	return target;
}

QRgb DecayColor(QRgb color, QRgb target, int decay)
{
	return qRgb(
		DecayComponent(qRed(color), qRed(target), cDecay),
		DecayComponent(qGreen(color), qGreen(target), cDecay),
		DecayComponent(qBlue(color), qBlue(target), cDecay)
	);
}

void EdvsVisual::Update()
{
	std::vector<Edvs::Event> events;
	{
		boost::interprocess::scoped_lock<boost::mutex> lock(events_mtx_);
		events = events_;
		events_.clear();
	}
	if(events.size() > 1000) {
		events.erase(events.begin() + 1000, events.end());
	}

	// write events
	{
		for(const Edvs::Event& e : events) {
			image_.setPixel(e.x, e.y, e.parity ? cColorOn : cColorOff);
		}
	}

	// write hough
	{
		for(const Edvs::Event& e : events) {
			cross_model->add_event(e.x, e.y, e.parity);
			// for(int ai=0; ai<HoughSizeAlpha; ai++) {
			// 	float a = impl::map_i2a(ai);
			// 	float d = static_cast<float>(e.x)*std::cos(a) + static_cast<float>(e.y)*std::sin(a);
			// 	int di = impl::map_d2i(d);
			// 	hough_(ai, di) += 1.0f;
			// }
		}
	}
}

void EdvsVisual::Display()
{
	// vis events
	{
		unsigned int* bits = (unsigned int*)image_.bits();
		const unsigned int N = image_.height() * image_.width();
		for(int i=0; i<N; i++, bits++) {
			*bits = DecayColor(*bits, cColorMid, cDisplayInterval*cDecay);
		}
	}
	// vis hough
	{
		unsigned int* bits = (unsigned int*)img_hough_.bits();
		float* src = (float*)hough_.data();
		const float hough_max = hough_.maxCoeff();
		const unsigned int N = img_hough_.height() * img_hough_.width();
		for(int i=0; i<N; i++, ++bits, ++src) {
			*src *= 0.90f;
			int v = std::min(255, static_cast<int>(*src / hough_max * 255.0f));
			*bits = qRgb(v,v,v);
		}
	}
	// compute cross
	{
		cross_ = Eigen::MatrixXf::Constant(RetinaSize, RetinaSize, 0.0f);
		for(int ai=0; ai<HoughSizeAlpha; ++ai) {
			for(int di=0; di<HoughSizeDist; ++di) {
				for(int x=0; x<RetinaSize; ++x) {
					for(int y=0; y<RetinaSize; ++y) {

					}
				}
			}
		}
	}
	// vis cross
	{
		Eigen::MatrixXf cc = cross_model->compute_propability();
		cross_model->decay();
		float* src = (float*)cc.data();
		img_cross_ = QImage(cc.rows(), cc.cols(), QImage::Format_RGB32);
		unsigned int* bits = (unsigned int*)img_cross_.bits();
		unsigned int* bits_event = (unsigned int*)image_.bits();
		const unsigned int N = img_cross_.height() * img_cross_.width();
		for(int i=0; i<N; i++, ++bits, ++src) {
			int v = std::min(255, static_cast<int>(*src * 255.0f));
			*bits = qRgb(v,v,v);
			// if(bits_event) {
			// 	int v = qRed(*(bits_event++));
			// 	if(std::abs(v - 128) > 92) {
			// 		*bits = qRgb(255,0,0);
			// 	}
			// }
		}
	}
	// rescale so that we see more :) and display
	ui.label->setPixmap(QPixmap::fromImage(image_.scaled(cDisplaySize, cDisplaySize)));
	ui.label_2->setPixmap(QPixmap::fromImage(img_hough_.scaled(HoughSizeAlpha, HoughSizeDist)));
	ui.label_3->setPixmap(QPixmap::fromImage(img_cross_.scaled(cDisplaySize, cDisplaySize)));
}
