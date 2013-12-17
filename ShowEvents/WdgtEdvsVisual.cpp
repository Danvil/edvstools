#include "WdgtEdvsVisual.h"
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <time.h>


const unsigned int RetinaSize = 128;
const int cDecay = 24;
const int cDisplaySize = 128;
const int cUpdateInterval = 10;
const int cDisplayInterval = 20;

// blue/yellow color scheme
// const QRgb cColorMid = qRgb(0, 0, 0);
// const QRgb cColorOn = qRgb(255, 255, 0);
// const QRgb cColorOff = qRgb(0, 0, 255);

// black/white color scheme
const QRgb cColorMid = qRgb(128, 128, 128);
const QRgb cColorOn = qRgb(255, 255, 255);
const QRgb cColorOff = qRgb(0, 0, 0);

EdvsVisual::EdvsVisual(const Edvs::EventStream& dh, QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	image_ = QImage(RetinaSize, RetinaSize, QImage::Format_RGB32);

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
	// write events
	{
		boost::interprocess::scoped_lock<boost::mutex> lock(events_mtx_);
		for(const Edvs::Event& e : events_) {
			image_.setPixel(e.x, 127-e.y, e.parity ? cColorOn : cColorOff);
		}
		events_.clear();
	}
}

void EdvsVisual::Display()
{
	// apply decay
	unsigned int* bits = (unsigned int*)image_.bits();
	const unsigned int N = image_.height() * image_.width();
	for(int i=0; i<N; i++, bits++) {
		*bits = DecayColor(*bits, cColorMid, cDisplayInterval*cDecay);
	}
	// rescale so that we see more :) and display
	ui.label->setPixmap(QPixmap::fromImage(image_.scaled(cDisplaySize, cDisplaySize)));
}
