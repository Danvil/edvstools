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
	EdvsVisual(const Edvs::EventStream& dh, QWidget *parent = 0);
	~EdvsVisual();

	void OnEvent(const std::vector<Edvs::Event>& events);

public Q_SLOTS:
	void Update();

private:
	Edvs::EventStream edvs_event_stream_;
	Edvs::EventCapture edvs_event_capture_;
	std::vector<Edvs::Event> events_;
	boost::mutex events_mtx_;
	QTimer timer_;
	QImage image_;

private:
    Ui::EdvsVisualClass ui;
};

#endif
