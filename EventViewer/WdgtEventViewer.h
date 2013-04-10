#ifndef WDGTEVENTVIEWER_H
#define WDGTEVENTVIEWER_H

#include <Edvs/Event.h>
#include <Edvs/Tools/Omnirob.hpp>
#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include "ui_WdgtEventViewer.h"
#include <vector>
#include <utility>

class WdgtCameraParameters;

class WdgtEventViewer : public QWidget
{
    Q_OBJECT

public:
    WdgtEventViewer(QWidget *parent = 0);
    ~WdgtEventViewer();

    void loadEventFile(const std::string& fn);

    void play();

    void enableOmnirobMode();

    void createVideo(const std::string& path, uint64_t dt_mus);

private:
	std::pair<std::vector<Edvs::Event>::const_iterator,std::vector<Edvs::Event>::const_iterator> findEventRange() const;

public Q_SLOTS:
	void onClickedLoad();
	void onClickedVideo();
	void onClickedPlay(bool checked);
	void onChangedEvent(int val);
	void onChangedTime(int val);
	void onChangedFalloff(int val);
	void onClickedWindow();
	void onTick();

private:
	std::size_t findEventByTime(uint64_t time) const;
	uint64_t getEventTime(std::size_t id) const;

	void updateGui();

	QImage createEventImage(const std::pair<std::vector<Edvs::Event>::const_iterator,std::vector<Edvs::Event>::const_iterator>& range);

	void paintEvents();

private:
	Ui::WdgtEventViewerClass ui;
	QTimer timer_;

	std::vector<Edvs::Event> events_;
	uint64_t time_;
	std::size_t event_id_;
	bool is_playing_;

	bool is_omnirob_;
	Edvs::Omnirob::EventMapper omnirob_params_;

	WdgtCameraParameters* wdgt_cam_params_;

};

#endif
