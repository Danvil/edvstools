#ifndef WdgtCameraParameters_H
#define WdgtCameraParameters_H

#include <Edvs/Event.hpp>
#include "Omnirob.hpp"
#include <QtGui/QWidget>
#include "ui_WdgtCameraParameters.h"

class WdgtCameraParameters
: public QWidget
{
Q_OBJECT

public:
	WdgtCameraParameters(QWidget *parent = 0);
	
	~WdgtCameraParameters();

	const std::vector<Edvs::EdvsSensorModelF>& getParams() const {
		return params_;
	}

	struct GuiParams
	{
		float fov;
		float omni_fov;
		float kappa_1;
		float kappa_2;
		float omni_altitude;
		float omni_dist;
		float azimuth_top;
		float azimuth_1;
		float azimuth_2;
		float azimuth_3;
		float azimuth_4;
		float azimuth_5;
		float azimuth_6;
	};

	GuiParams fromGui() const;
	void toGui(const GuiParams& p);

	void save(const std::string& filename);
	void load(const std::string& filename);

	bool dirty;

public Q_SLOTS:
	void onSave();
	void onLoad();
	void valueChanged();

private:
	Ui::WdgtCameraParametersClass ui;

	std::vector<Edvs::EdvsSensorModelF> params_;

};

#endif
