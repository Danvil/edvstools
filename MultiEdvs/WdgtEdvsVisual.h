#ifndef WDGTEDVSVISUAL_H
#define WDGTEDVSVISUAL_H

#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include "ui_WdgtEdvsVisual.h"
#include <Edvs/EventStream.hpp>
#include <Edvs/EventCapture.hpp>
#include <boost/thread.hpp>
#include <vector>

class EdvsVisual : public QWidget
{
    Q_OBJECT

public:
	EdvsVisual(const Edvs::EventStream& dh1, const Edvs::EventStream& dh2, QWidget *parent = 0);
	~EdvsVisual();

	void OnEvent(const std::vector<Edvs::Event>& events, uint8_t id);

public Q_SLOTS:
	void OnButton();

	void Update();
	void Display();

private:
	Edvs::EventStream edvs_event_stream_1_;
	Edvs::EventStream edvs_event_stream_2_;
	Edvs::EventCapture edvs_event_capture_1_;
	Edvs::EventCapture edvs_event_capture_2_;
	std::vector<Edvs::Event> events_;
	boost::mutex events_mtx_;
	QTimer timer_update_;
	QTimer timer_display_;
	QImage image_;

	bool is_recording_;
	std::vector<Edvs::Event> events_recorded_;

private:
    Ui::EdvsVisualClass ui;
};

#endif
