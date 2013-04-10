#include "WdgtEventViewer.h"
#include "WdgtCameraParameters.h"
#include <Edvs/Tools/IO.hpp>
#include <QtGui/QFileDialog>
#include <boost/format.hpp>
#include <boost/progress.hpp>
#include <algorithm>

constexpr int OMNIROB_CNT = 1;

WdgtEventViewer::WdgtEventViewer(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pushButtonLoad, SIGNAL(clicked()), this, SLOT(onClickedLoad()));
	connect(ui.pushButtonVideo, SIGNAL(clicked()), this, SLOT(onClickedVideo()));
	connect(ui.pushButtonPlay, SIGNAL(clicked(bool)), this, SLOT(onClickedPlay(bool)));
	connect(ui.horizontalSliderEvent, SIGNAL(valueChanged(int)), this, SLOT(onChangedEvent(int)));
	connect(ui.horizontalSliderTime, SIGNAL(valueChanged(int)), this, SLOT(onChangedTime(int)));
	connect(ui.radioButtonWindowsEvent, SIGNAL(clicked()), this, SLOT(onClickedWindow()));
	connect(ui.radioButtonWindowsTime, SIGNAL(clicked()), this, SLOT(onClickedWindow()));
	connect(ui.spinBoxFalloff, SIGNAL(valueChanged(int)), this, SLOT(onChangedFalloff(int)));
	connect(&timer_, SIGNAL(timeout()), this, SLOT(onTick()));

	ui.horizontalSliderTime->setEnabled(false);
	ui.horizontalSliderEvent->setEnabled(false);
	ui.pushButtonPlay->setEnabled(false);
	is_playing_ = false;

	is_omnirob_ = false;

	timer_.setInterval(1);
	timer_.start();

	wdgt_cam_params_ = new WdgtCameraParameters();
	wdgt_cam_params_->show();
}

WdgtEventViewer::~WdgtEventViewer()
{

}

void WdgtEventViewer::loadEventFile(const std::string& fn)
{
	boost::timer timer;
	if(is_omnirob_) {
		boost::format fn_fmt(fn + "-%1%.txt");
		events_.clear();
		for(unsigned int i=0; i<OMNIROB_CNT; i++) {
			std::string fni = (fn_fmt % i).str();
			std::cout << "Loading '" << fni << "'... " << std::flush;
			std::vector<Edvs::Event> events = Edvs::LoadEvents(fni, Edvs::EventFileFlag::RawWithId);
			std::cout << events.size() << std::endl;
			events_.insert(events_.end(), events.begin(), events.end());
		}
		std::cout << "Sorting events..." << std::endl;
		std::sort(events_.begin(), events_.end(),
			[](const Edvs::Event& x, const Edvs::Event& y) {
				return x.time < y.time;
			}
		);
	}
	else {
		std::cout << "Loading '" << fn << "'... " << std::endl;
		events_ = Edvs::LoadEvents(fn, Edvs::EventFileFlag::RawWithId);
	}
	std::cout << "Loaded " << events_.size() << " events in " << timer.elapsed() << " s" << std::endl;
	ui.horizontalSliderTime->setMinimum(events_.front().time/1000);
	ui.horizontalSliderTime->setMaximum(events_.back().time/1000);
	ui.horizontalSliderEvent->setMinimum(0);
	ui.horizontalSliderEvent->setMaximum(events_.size() - 1);
	ui.pushButtonPlay->setEnabled(true);
	ui.horizontalSliderTime->setEnabled(true);
	ui.horizontalSliderEvent->setEnabled(true);
	time_ = 0;
}

void WdgtEventViewer::play()
{
	if(events_.empty()) {
		std::cerr << "Error: Need to load a valid events file before playing!" << std::endl;
		return;
	}
	ui.pushButtonPlay->setChecked(true);
	is_playing_ = true;
}

void WdgtEventViewer::enableOmnirobMode()
{
	is_omnirob_ = true;
}

void WdgtEventViewer::createVideo(const std::string& path, uint64_t dt_mus)
{
	std::cout << "Creating video in '" << path << "'..." << std::endl;
	unsigned int frame_count = 0;
	boost::format fn_fmt(path + "_%05d.png");
	auto it = events_.begin();
	while(true) {
		auto next = std::lower_bound(it, events_.end(), it->time + dt_mus,
				[](const Edvs::Event& e, uint64_t v) { return e.time < v; });
		auto next2 = std::lower_bound(it, events_.end(), it->time + 5*dt_mus,
				[](const Edvs::Event& e, uint64_t v) { return e.time < v; });
		if(next == events_.end()) {
			break;
		}
		QImage img = createEventImage({it, next2});
		std::string fn = (fn_fmt % frame_count).str();
		img.save(QString::fromStdString(fn));
		it = next;
		frame_count++;
	}
	std::cout << "Wrote " << frame_count << " video frames" << std::endl;
	std::cout << "Run the following command to create the video: " << std::endl;
	std::cout << "ffmpeg -f image2 -i " << path << "_%05d.png -b 10000k edvs_video.mpg" << std::endl;
}

void WdgtEventViewer::onClickedLoad()
{
	QString fn = QFileDialog::getOpenFileName(this, "Select event file");
	if(fn.isEmpty()) {
		return;
	}
	loadEventFile(fn.toStdString());
}

void WdgtEventViewer::onClickedVideo()
{
	createVideo("/tmp/edvs_video", 10000);
}

void WdgtEventViewer::onClickedPlay(bool checked)
{
	is_playing_ = checked;
}

void WdgtEventViewer::onChangedEvent(int val)
{
	event_id_ = val;
	time_ = getEventTime(event_id_);
	updateGui();
	paintEvents();
}

void WdgtEventViewer::onChangedTime(int val)
{
	time_ = val * 1000;
	event_id_ = findEventByTime(time_);
	updateGui();
	paintEvents();
}

void WdgtEventViewer::onChangedFalloff(int val)
{
	paintEvents();
}

void WdgtEventViewer::onClickedWindow()
{
	paintEvents();
}

void WdgtEventViewer::onTick()
{
	if(is_playing_) {
		ui.horizontalSliderTime->setValue(time_/1000 + 10);
	}
	if(wdgt_cam_params_->dirty) {
		paintEvents();
		wdgt_cam_params_->dirty = false;
	}
}

std::size_t WdgtEventViewer::findEventByTime(uint64_t time) const
{
	auto it = std::lower_bound(events_.begin(), events_.end(), time,
			[](const Edvs::Event& e, uint64_t v) { return e.time < v; });
	if(it == events_.end()) {
		return events_.size() - 1;
	}
	else {
		return std::distance(events_.begin(), it);
	}
}

uint64_t WdgtEventViewer::getEventTime(std::size_t id) const
{
	return events_[id].time;
}

void WdgtEventViewer::updateGui()
{
	ui.horizontalSliderEvent->blockSignals(true);
	ui.horizontalSliderEvent->setValue(event_id_);
	ui.horizontalSliderEvent->blockSignals(false);

	ui.horizontalSliderTime->blockSignals(true);
	ui.horizontalSliderTime->setValue(time_/1000);
	ui.horizontalSliderTime->blockSignals(false);

	ui.labelEvent->setText(QString("%1 []").arg(event_id_, 6, 10, QChar('0')));
	ui.labelTime->setText(QString("%1 [ms]").arg(time_/1000, 5, 10, QChar('0')));

//	std::cout << "Event=" << event_id_ << ", Time=" << time_ << std::endl;

}

std::pair<std::vector<Edvs::Event>::const_iterator,std::vector<Edvs::Event>::const_iterator> WdgtEventViewer::findEventRange() const
{
	std::vector<Edvs::Event>::const_iterator it_begin, it_end;
	if(ui.radioButtonWindowsEvent->isChecked()) {
		const unsigned int de = (is_omnirob_ ? OMNIROB_CNT : 1) * ui.spinBoxFalloff->value();
		it_begin = events_.begin() + ((event_id_ < de) ? 0 : (event_id_ - de));
		it_end = events_.begin() + event_id_;
	}
	else {
		const unsigned int dt = ui.spinBoxFalloff->value() * 1000; // mus
		uint64_t time_begin = (time_ < dt) ? 0 : time_ - dt;
		uint64_t time_end = time_;
		it_begin = std::lower_bound(events_.begin(), events_.end(), time_begin,
				[](const Edvs::Event& e, uint64_t v) { return e.time < v; });
		it_end = std::lower_bound(it_begin, events_.cend(), time_end,
				[](const Edvs::Event& e, uint64_t v) { return e.time < v; });
	}
	return { it_begin, it_end };
}

QImage WdgtEventViewer::createEventImage(const std::pair<std::vector<Edvs::Event>::const_iterator,std::vector<Edvs::Event>::const_iterator>& it_range)
{
	// create image
	QImage img;
	if(is_omnirob_) {
		omnirob_params_.setCameraParameters(wdgt_cam_params_->getParams());
		// // print
		// for(unsigned int i=0; i<OMNIROB_CNT; i++) {
		// 	auto& q = omnirob_params_.getCameraParameters()[i];
		// 	std::cout << "position : " << q.position.transpose() << std::endl;
		// 	std::cout << "rotation : " << q.rotation << std::endl;
		// 	std::cout << "angle : " << q.getOpeningAngle() << std::endl;
		// }
		QRgb colors[OMNIROB_CNT] = {
			qRgb(255,0,0),
			// qRgb(255,255,0),
			// qRgb(0,255,0),
			// qRgb(0,255,255),
			// qRgb(0,0,255),
			// qRgb(255,0,255),
			// qRgb(255,255,255)
		};
		constexpr int SIZE = 512;
		constexpr float SCALE = 200.0f * static_cast<float>(SIZE) / 512.0f;
		constexpr float COMPRESS = 0.1f;
		// // prepare image
		img = QImage(SIZE, SIZE, QImage::Format_ARGB32);
		img.fill(qRgb(0,0,0));
		// paint
		for(auto it=it_range.first; it!=it_range.second; ++it) {
			const Edvs::Event& event = *it;
			Edvs::PixelViewCone pvc = omnirob_params_.map(event);
			Eigen::Vector2f u = Edvs::Omnirob::StereographicProjection(pvc.ray_dir);
			u *= SCALE / (1.0f + COMPRESS*u.norm());
			int px = static_cast<int>(u.x()) + SIZE/2;
			int py = static_cast<int>(u.y()) + SIZE/2;
			if(0 <= px && px < SIZE && 0 <= py && py < SIZE) {
				img.setPixel(px, py, colors[event.id]);
			}
		}
		for(unsigned int i=0; i<OMNIROB_CNT; i++) {
			Edvs::Event event;
			event.id = i;
			event.x = 0;
			event.y = 0;
			Edvs::PixelViewCone pvc = omnirob_params_.map(event);
			Eigen::Vector2f u = Edvs::Omnirob::StereographicProjection(pvc.ray_dir);
			int px = static_cast<int>(SCALE*u.x()) + SIZE/2;
			int py = static_cast<int>(SCALE*u.y()) + SIZE/2;
			if(0 <= px && px < SIZE && 0 <= py && py < SIZE) {
				img.setPixel(px, py, qRgb(128,128,128));
			}
//			img.setPixel(SIZE/2, SIZE/2, qRgb(255,0,0));
		}
	}
	else {
		// prepare image
		img = QImage(128, 128, QImage::Format_ARGB32);
		img.fill(qRgb(0,0,0));
		// paint
		int64_t time_start = it_range.first->time;
		int64_t timespan = (it_range.second == it_range.first ? 0 : (std::prev(it_range.second)->time - time_start));
		for(auto it=it_range.first; it!=it_range.second; ++it) {
			const Edvs::Event& event = *it;
			int64_t dt = event.time - time_start;
			float p = std::min(1.0f, static_cast<float>(dt) / static_cast<float>(timespan));
			unsigned char pc = static_cast<unsigned char>(255.0f * p);
			int px = static_cast<int>(event.x);
			int py = static_cast<int>(event.y);
			if(0 <= px && px < 128 && 0 <= py && py < 128) {
				img.setPixel(px, py, qRgb(pc,pc,255-pc));
			}
		}
		img.setPixel(0, 0, qRgb(255,0,0));
	}
	return img;
}

void WdgtEventViewer::paintEvents()
{
	// find range
	auto it_range = findEventRange();
	// create image
	QImage img = createEventImage(it_range);
	if(img.width() == 128 && img.height() == 128) {
		// scale
		img = img.scaled(512,512);
	}
	// display
	ui.labelEvents->setPixmap(QPixmap::fromImage(img));
	ui.labelEvents->adjustSize();
}
