#include "WdgtEdvsVisual.h"
#include <boost/bind.hpp>
#include <iostream>


const int cDecay = (2 * 256) / 60;
const int cDisplaySize = 512;
const int cUpdateInterval = 0;
const bool cUseNetSocket = true; 
const Edvs::Baudrate cBaudrate = Edvs::B4000k;
const char* cDeviceName = "/dev/ttyUSB0";
const char* cAdresse = "192.168.201.62:56001";
const QRgb cColorOn = qRgb(255, 255, 0);
const QRgb cColorOff = qRgb(0, 0, 255);

EdvsVisual::EdvsVisual(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	image_ = QImage(Edvs::cDeviceDisplaySize, Edvs::cDeviceDisplaySize, QImage::Format_RGB32);

	timer_.connect(&timer_, SIGNAL(timeout()), this, SLOT(Update()));
	timer_.setInterval(cUpdateInterval);
	timer_.start();

	// open device and start capture
	if(cUseNetSocket)
	{
		device_ = Edvs::create_network_device(cAdresse);
	}
	else
	{
		device_ = Edvs::create_serial_device(cBaudrate, cDeviceName);
	}
	capture_ = Edvs::EventCapture(device_, boost::bind(&EdvsVisual::OnEvent, this, _1));
}

EdvsVisual::~EdvsVisual()
{
}

QRgb Add(QRgb a, QRgb b) {
	return qRgb(
			std::min(qRed(a) + qRed(b), 255),
			std::min(qGreen(a) + qGreen(b), 255),
			std::min(qBlue(a) + qBlue(b), 255)
	);
}

void EdvsVisual::OnEvent(const std::vector<Edvs::RawEvent>& events)
{
	// this functions is not called in the gui thread, so we shall not use gui objects!
	for(std::vector<Edvs::RawEvent>::const_iterator it=events.begin(); it!=events.end(); it++) {
		// an event is translated into a color dot
		QRgb color = Add(image_.pixel(it->x, it->y), it->parity ? cColorOn : cColorOff);
		image_.setPixel(it->x, it->y, color);
	}
}

void EdvsVisual::Update()
{
	// apply decay
	// FIXME make if faster by not using pixel/setPixel!
	for(int y=0; y<image_.height(); y++) {
		for(int x=0; x<image_.width(); x++) {
			QRgb color = image_.pixel(x, y);
			int cr = qRed(color) - cDecay;
			int cg = qGreen(color) - cDecay;
			int cb = qBlue(color) - cDecay;
			color = qRgb(cr < 0 ? 0 : cr, cg < 0 ? 0 : cg, cb < 0 ? 0 : cb);
			image_.setPixel(x, y, color);
		}
	}
	// rescale so that we see more :)
	QImage big = image_.scaled(cDisplaySize, cDisplaySize);
	// display
	ui.label->setPixmap(QPixmap::fromImage(big));
}
