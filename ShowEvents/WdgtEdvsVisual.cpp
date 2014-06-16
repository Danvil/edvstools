#include "WdgtEdvsVisual.h"
#include <Edvs/EventIO.hpp>
#include <QtGui/QFileDialog>
#include <iostream>


const unsigned int RetinaSize = 128;
const int cDecay = 24;
const int cDisplaySize = 4*128;
const int cUpdateInterval = 0;
const int cDisplayInterval = 20;

// blue/yellow color scheme
// const QRgb cColorMid = qRgb(0, 0, 0);
// const QRgb cColorOn = qRgb(255, 255, 0);
// const QRgb cColorOff = qRgb(0, 0, 255);

// black/white color scheme
const QRgb cColorMid = qRgb(128, 128, 128);
const QRgb cColorOn = qRgb(255, 255, 255);
const QRgb cColorOff = qRgb(0, 0, 0);

EdvsVisual::EdvsVisual(const std::shared_ptr<Edvs::IEventStream>& stream, QWidget *parent)
:	QWidget(parent),
	edvs_event_stream_(stream)
{
	ui.setupUi(this);

	connect(ui.pushButtonRecord, SIGNAL(clicked()), this, SLOT(OnButton()));
	is_recording_ = false;

	connect(&timer_update_, SIGNAL(timeout()), this, SLOT(Update()));
	timer_update_.setInterval(cUpdateInterval);
	timer_update_.start();

	connect(&timer_display_, SIGNAL(timeout()), this, SLOT(Display()));
	timer_display_.setInterval(cDisplayInterval);
	timer_display_.start();
}

EdvsVisual::~EdvsVisual()
{}

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

void EdvsVisual::Update()
{
	// read events
	auto events = edvs_event_stream_->read();

	if(is_recording_) {
		events_recorded_.insert(events_recorded_.end(), events.begin(), events.end());
	}

	// print time information
	if(!events.empty()) {
		static uint64_t last_time = 0;
		uint64_t current_time = events.back().t;
		if(current_time >= last_time + 1000000) {
			std::cout << static_cast<float>(current_time)/1000000.0f << std::endl;
			last_time += 1000000;
		}
	}

	// write events to image
	for(const Edvs::Event& e : events) {
		if(e.id >= items_.size() || !items_[e.id].label) {
			addItem(e.id);
		}
		items_[e.id].image.setPixel(e.x, e.y, e.parity ? cColorOn : cColorOff);
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

void EdvsVisual::Display()
{
	for(const Item& i : items_) {
		// apply decay
		unsigned int* bits = (unsigned int*)i.image.bits();
		const unsigned int N = i.image.height() * i.image.width();
		for(int i=0; i<N; i++, bits++) {
			*bits = DecayColor(*bits, cColorMid, cDisplayInterval*cDecay);
		}
		// rescale so that we see more :) and display
		i.label->setPixmap(QPixmap::fromImage(i.image.scaled(cDisplaySize, cDisplaySize)));
	}
}

void EdvsVisual::addItem(uint8_t id)
{
	items_.resize(id+1);
	items_[id].image = QImage(RetinaSize, RetinaSize, QImage::Format_RGB32);
	QLabel* label = new QLabel();
	ui.gridLayout->addWidget(label, 0, id);
	items_[id].label = label;
}
