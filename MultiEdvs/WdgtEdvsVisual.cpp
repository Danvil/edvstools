#include "WdgtEdvsVisual.h"
#include <QFileDialog>
#include <Edvs/EventIO.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <time.h>


const unsigned int RetinaSize = 128;
const int cDecay = 24;
const int cDisplaySize = 512;
const int cUpdateInterval = 10;
const int cDisplayInterval = 20;

// blue/yellow color scheme
// const QRgb cColorMid = qRgb(0, 0, 0);
// const QRgb cColorOn = qRgb(255, 255, 0);
// const QRgb cColorOff = qRgb(0, 0, 255);

// black/white color scheme
const QRgb cColorMid = qRgb(0, 0, 0);
const QRgb cColorOn1 = qRgb(255, 0, 0);
const QRgb cColorOff1 = cColorOn1;//qRgb(0, 255, 0);
const QRgb cColorOn2 = qRgb(0, 255, 255);
const QRgb cColorOff2 = cColorOn2;//qRgb(255, 128, 0);

EdvsVisual::EdvsVisual(const Edvs::EventStream& stream, QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	image_ = QImage(RetinaSize, RetinaSize, QImage::Format_RGB32);

	connect(ui.pushButtonRecord, SIGNAL(clicked()), this, SLOT(OnButton()));
	is_recording_ = false;

	connect(&timer_update_, SIGNAL(timeout()), this, SLOT(Update()));
	timer_update_.setInterval(cUpdateInterval);
	timer_update_.start();

	connect(&timer_display_, SIGNAL(timeout()), this, SLOT(Display()));
	timer_display_.setInterval(cDisplayInterval);
	timer_display_.start();

	// start capture
	edvs_event_stream_ = stream;
	edvs_event_capture_ = Edvs::EventCapture(edvs_event_stream_,
		boost::bind(&EdvsVisual::OnEvent, this, _1));
}

EdvsVisual::~EdvsVisual()
{
	edvs_event_capture_.stop();
}

void EdvsVisual::OnButton()
{
	if(ui.pushButtonRecord->isChecked()) {
		// start recording
		ui.pushButtonRecord->setText("Recording... (press to stop)");
		is_recording_ = true;
	}
	else {
		// stop recording
		ui.pushButtonRecord->setText("Start recording");
		is_recording_ = false;
		// get filename
		QString fn = QFileDialog::getSaveFileName(this, "Select file to save recording");
		if(fn != "") {
			Edvs::SaveEvents(fn.toStdString(), events_recorded_);
			std::cout << "Saved " << events_recorded_.size() << " events to file '" << fn.toStdString() << "'" << std::endl;
		}
		events_recorded_.clear();
	}
}

void EdvsVisual::OnEvent(const std::vector<Edvs::Event>& newevents)
{
	// protect common vector with a mutex to avoid race conditions
	boost::interprocess::scoped_lock<boost::mutex> lock(events_mtx_);
	events_.insert(events_.end(), newevents.begin(), newevents.end());
	if(is_recording_) {
		events_recorded_.insert(events_recorded_.end(), newevents.begin(), newevents.end());
	}

	// std::cout << newevents.size() << std::endl;
	// for(const auto& e : newevents) {
	// 	std::cout << e.t << " " << static_cast<int>(e.id) << std::endl;
	// }

	// print time information
	static uint64_t last_time = 0;
	uint64_t current_time = newevents.back().t;
	if(current_time >= last_time + 1000000) {
		std::cout << "Current time: " << static_cast<float>(current_time)/1000000.0f << std::endl;
		last_time = current_time;
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
			image_.setPixel(e.x, 127-e.y,
				(e.id == 0)
				? (e.parity ? cColorOn1 : cColorOff1)
				: (e.parity ? cColorOn2 : cColorOff2)
			);
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
