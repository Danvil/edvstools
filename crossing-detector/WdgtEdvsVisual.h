#ifndef WDGTEDVSVISUAL_H
#define WDGTEDVSVISUAL_H

#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include "ui_WdgtEdvsVisual.h"
#include <Edvs/EventStream.hpp>
#include <Edvs/EventCapture.hpp>
#include <Eigen/Dense>
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
	void Display();

private:
	Edvs::EventStream edvs_event_stream_;
	Edvs::EventCapture edvs_event_capture_;
	std::vector<Edvs::Event> events_;
	boost::mutex events_mtx_;
	QTimer timer_update_;
	QTimer timer_display_;
	QImage image_;
	QImage img_hough_;
	QImage img_cross_;

	Eigen::MatrixXf hough_;
	Eigen::MatrixXf cross_;

private:
    Ui::EdvsVisualClass ui;
};

#endif
