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
	static constexpr int NA = 8;

	int r;

	std::vector<Eigen::MatrixXf> t;
	std::vector<Eigen::MatrixXf> data;

	CrossModel(float sigma=20.0f) {
		r = std::max<int>(3, std::ceil(2.0f * sigma)); // TODO 3.0f is only an approximation
		int a = 2*r+1;

		QImage dbg(8*a, NA/8*a, QImage::Format_RGB32);

		t = std::vector<Eigen::MatrixXf>(NA, Eigen::MatrixXf::Zero(a,a));
		// for(int x=-r; x<=+r; x++) {
		// 	for(int y=-r; y<=+r; y++) {
		// 		float xf = static_cast<float>(x);
		// 		float yf = static_cast<float>(y);
		// 		if(x == 0 && y == 0) continue;
		// 		float angle = static_cast<float>(NA) * (std::atan2(yf,xf) + PI) / PI;
		// 		int ai = static_cast<int>(angle);
		// 		float ap = angle - static_cast<float>(ai);
		// 		const float d2 = xf*xf + yf*yf;
		// 		const float f = std::exp(-0.5f*d2/(sigma*sigma)) / std::sqrt(d2);
		// 		t[(ai+0)%NA](x+r,y+r) += f * (1.0f - ap);
		// 		t[(ai+1)%NA](x+r,y+r) += f * ap;
		// 	}
		// }
		// for(int ai=0; ai<NA; ai++) {
		// 	t[ai](r,r) = 1.0f / static_cast<float>(NA);
		// }
		for(int ai=0; ai<NA; ai++) {
			const float delta_angle = 1.0f / static_cast<float>(NA) * PI;
			const float angle_base = static_cast<float>(ai) * delta_angle;
			for(int x=-r; x<=+r; x++) {
				for(int y=-r; y<=+r; y++) {					
					float xf = static_cast<float>(x);
					float yf = static_cast<float>(y);
					if(x == 0 && y == 0) continue;
					float angle = std::atan2(yf,xf);
					float da = angle - angle_base;
					while(da < 0) da += PI;
					while(da > PI) da -= PI;
					if(da > 0.5f*PI) da = PI - da;
					const float d2 = xf*xf + yf*yf;
					const float d = std::sqrt(d2);
					const float num_px = d * 2.0f * PI;
					const float px_da = 1.0f / num_px * 2.0f * PI;
					float dap = std::max<float>(0.0f, 1.0f - da / std::max<float>(px_da, delta_angle));
					const float f = std::exp(-0.5f*d2/(sigma*sigma));
					t[ai](x+r,y+r) += f * dap;// / num_px;
				}
			}
			t[ai](r,r) = 1.0f / static_cast<float>(NA);
		}

		{
			float scl = t[0].maxCoeff();
			for(int ai=0; ai<NA; ai++) {
				scl = std::max(scl, t[ai].maxCoeff());
			}
			for(int ai=0; ai<NA; ai++) {
				int dx = ai % 8;
				int dy = ai / 8;
				for(int y=0; y<a; y++) {
					for(int x=0; x<a; x++) {
						int v = std::min(255, static_cast<int>(t[ai](x,y) / scl * 255.0f));
						dbg.setPixel(dx*a + x, dy*a + y, qRgb(v,v,v));
					}
				}
			}
			QLabel* label1 = new QLabel();
			label1->setPixmap(QPixmap::fromImage(dbg));
			label1->show();
		}

		{
			QImage dbg2(a, a, QImage::Format_RGB32);
			Eigen::MatrixXf sum = t[0];
			for(int ai=1; ai<NA; ai++) {
				sum += t[ai];
			}
			float scl = sum.maxCoeff();
			for(int y=0; y<a; y++) {
				for(int x=0; x<a; x++) {
					int v = std::min(255, static_cast<int>(sum(x,y) / scl * 255.0f));
					dbg2.setPixel(x, y, qRgb(v,v,v));
				}
			}
			QLabel* label2 = new QLabel();
			label2->setPixmap(QPixmap::fromImage(dbg2));
			label2->show();
		}

		data = std::vector<Eigen::MatrixXf>(NA, Eigen::MatrixXf::Zero(S,S));
	}

	void add_event(int x, int y) {
		// x = y = 64;
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
		// FIXME add rotational dependency
		for(int ai=0; ai<NA; ai++) {
			data[ai].block(ax1, ay1, sx, sy)
				+= t[ai].block(
					(x1 < 0) ? (ax1 - x1) : 0,
					(y1 < 0) ? (ay1 - y1) : 0,
					sx, sy);
		}
	}

	void decay() {
		for(int ai=0; ai<NA; ai++) {
			data[ai] *= 0.83f;
		}
	}

	Eigen::MatrixXf compute_propability() {
		Eigen::MatrixXf sum = Eigen::MatrixXf::Zero(S, S);
		std::vector<float> sums(data.size());
		for(int ai=0; ai<NA; ai++) {
			sums[ai] = data[ai].maxCoeff();
		}
		for(int x=0; x<S; ++x) {
			for(int y=0; y<S; ++y) {
				for(int ai=0; ai<NA; ai++) {
					sum(x,y) += data[ai](x,y)*data[(ai+NA/2)%NA](x,y)  / sums[ai]  / sums[(ai+NA/2)%NA];
				}
			}
		}
		return sum;
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
			cross_model->add_event(e.x, e.y);
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
		cross_model->decay();
		Eigen::MatrixXf cc = cross_model->compute_propability();
		float* src = (float*)cc.data();
		img_cross_ = QImage(cc.rows(), cc.cols(), QImage::Format_RGB32);
		unsigned int* bits = (unsigned int*)img_cross_.bits();
		const float cross_max = cc.maxCoeff();
		std::cout << cross_max << std::endl;
		const unsigned int N = img_cross_.height() * img_cross_.width();
		for(int i=0; i<N; i++, ++bits, ++src) {
			int v = std::min(255, static_cast<int>(*src / cross_max * 255.0f));
			*bits = qRgb(v,v,v);
		}
	}
	// rescale so that we see more :) and display
	ui.label->setPixmap(QPixmap::fromImage(image_.scaled(cDisplaySize, cDisplaySize)));
	ui.label_2->setPixmap(QPixmap::fromImage(img_hough_.scaled(HoughSizeAlpha, HoughSizeDist)));
	ui.label_3->setPixmap(QPixmap::fromImage(img_cross_.scaled(cDisplaySize, cDisplaySize)));
}
