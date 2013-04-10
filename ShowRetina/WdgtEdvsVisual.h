#ifndef WDGTEDVSVISUAL_H
#define WDGTEDVSVISUAL_H

#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include "ui_WdgtEdvsVisual.h"
#include <Edvs/EventCapture.hpp>
#include <boost/thread.hpp>
#include <vector>

class EdvsVisual : public QWidget
{
    Q_OBJECT

public:
	EdvsVisual(const Edvs::DeviceHandle& dh, QWidget *parent = 0);
	~EdvsVisual();

	void OnEvent(const std::vector<Edvs::RawEvent>& events);

public Q_SLOTS:
	void Update();

private:
	Edvs::DeviceHandle edvs_device_handle_;
	std::vector<Edvs::RawEvent> events_;
	boost::mutex events_mtx_;
	QTimer timer_;
	QImage image_;

private:
    Ui::EdvsVisualClass ui;
};

#endif
