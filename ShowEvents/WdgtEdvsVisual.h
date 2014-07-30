#ifndef WDGTEDVSVISUAL_H
#define WDGTEDVSVISUAL_H

#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include "ui_WdgtEdvsVisual.h"
#include <Edvs/EventStream.hpp>
#include <vector>

class EdvsVisual : public QWidget
{
    Q_OBJECT

public:
	EdvsVisual(const std::shared_ptr<Edvs::IEventStream>& stream, QWidget *parent = 0);
	~EdvsVisual();

public Q_SLOTS:
	void OnButton();
	void Update();
	void Display();

private:
	void addItem(uint8_t id);

private:
	std::shared_ptr<Edvs::IEventStream> edvs_event_stream_;
	QTimer timer_update_;
	QTimer timer_display_;

	struct Item {
		QImage image;
		QLabel* label;
	};

	std::vector<Item> items_;

	bool is_recording_;
	std::vector<Edvs::Event> events_recorded_;

private:
    Ui::EdvsVisualClass ui;
};

#endif
