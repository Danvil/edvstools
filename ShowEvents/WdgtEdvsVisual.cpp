#include "WdgtEdvsVisual.h"
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>


const unsigned int RetinaSize = 128;
const int cDecay = 4;
const int cDisplaySize = 512;
const int cUpdateInterval = 1;

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

	timer_.connect(&timer_, SIGNAL(timeout()), this, SLOT(Update()));
	timer_.setInterval(cUpdateInterval);
	timer_.start();

	// start capture
	edvs_event_stream_ = dh;
	edvs_event_capture_ = Edvs::EventCapture(edvs_event_stream_,
		std::bind(&EdvsVisual::OnEvent, this, std::placeholders::_1));
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

	// for(const auto& e : newevents) {
	// 	std::cout << e.t << ", ";
	// }
	// std::cout << std::endl;
	if(!newevents.empty())
		std::cout << newevents.back().t << std::endl;
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
	// apply decay
	// FIXME make if faster by not using pixel/setPixel!
	for(int y=0; y<image_.height(); y++) {
		for(int x=0; x<image_.width(); x++) {
			image_.setPixel(x, y, DecayColor(image_.pixel(x, y), cColorMid, cDecay));
		}
	}
	// write events
	{
		boost::interprocess::scoped_lock<boost::mutex> lock(events_mtx_);
		for(const Edvs::Event& e : events_) {
			image_.setPixel(e.x, e.y, e.parity ? cColorOn : cColorOff);
		}
		events_.clear();
	}
	// rescale so that we see more :)
	QImage big = image_.scaled(cDisplaySize, cDisplaySize);
	// display
	ui.label->setPixmap(QPixmap::fromImage(big));
}