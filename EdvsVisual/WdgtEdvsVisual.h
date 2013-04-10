#ifndef WDGTEDVSVISUAL_H
#define WDGTEDVSVISUAL_H

#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include "ui_WdgtEdvsVisual.h"
#include "Edvs.h"

class EdvsVisual : public QWidget
{
    Q_OBJECT

public:
	EdvsVisual(QWidget *parent = 0);
	~EdvsVisual();

	void OnEvent(const std::vector<Edvs::RawEvent>& events);

public Q_SLOTS:
	void Update();

private:
	boost::shared_ptr<Edvs::Device> device_;
	Edvs::EventCapture capture_;
	QTimer timer_;
	QImage image_;

private:
    Ui::EdvsVisualClass ui;
};

#endif
