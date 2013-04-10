#include "WdgtCameraParameters.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

constexpr int OMNIROB_CNT = 7;

WdgtCameraParameters::WdgtCameraParameters(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));
	connect(ui.pushButtonLoad, SIGNAL(clicked()), this, SLOT(onLoad()));
	connect(ui.horizontalSliderFOV, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
	connect(ui.horizontalSliderOmniFOV, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
	connect(ui.horizontalSliderKappa1, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
	connect(ui.horizontalSliderKappa2, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
	connect(ui.horizontalSliderOmniAngle, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
	connect(ui.horizontalSliderOmniDist, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
	connect(ui.horizontalSliderAngleTop, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
	connect(ui.doubleSpinBoxAngle1, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
	connect(ui.doubleSpinBoxAngle2, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
	connect(ui.doubleSpinBoxAngle3, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
	connect(ui.doubleSpinBoxAngle4, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
	connect(ui.doubleSpinBoxAngle5, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
	connect(ui.doubleSpinBoxAngle6, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));

	params_.resize(OMNIROB_CNT);

	valueChanged();
}

WdgtCameraParameters::~WdgtCameraParameters()
{

}

float convertParameter(int value, float min, float max) {
	float p = static_cast<float>(value) / 99.0f;
	return min + (max - min) * p;
}

int convertParameterToGui(float value, float min, float max) {
	float p = (value - min) / (max - min);
	p = std::max(0.0f, std::min(1.0f, p));
	return static_cast<int>(99.0f * p);
}

WdgtCameraParameters::GuiParams WdgtCameraParameters::fromGui() const
{
	GuiParams gp;
	gp.fov = convertParameter(ui.horizontalSliderFOV->value(), 45.0f, 85.0f);
	gp.omni_fov = convertParameter(ui.horizontalSliderOmniFOV->value(), 45.0f, 85.0f);
	gp.kappa_1 = convertParameter(ui.horizontalSliderKappa1->value(), 0.0f, 0.003f);
	gp.kappa_2 = convertParameter(ui.horizontalSliderKappa2->value(), 0.0f, 0.0003f);
	gp.omni_altitude = convertParameter(ui.horizontalSliderOmniAngle->value(), 0.0f, 60.0f);
	gp.omni_dist = convertParameter(ui.horizontalSliderOmniDist->value(), 0.03f, 0.10f);
	gp.azimuth_top = convertParameter(ui.horizontalSliderAngleTop->value(), 0.0f, 360.0f);
	gp.azimuth_1 = ui.doubleSpinBoxAngle1->value();
	gp.azimuth_2 = ui.doubleSpinBoxAngle2->value();
	gp.azimuth_3 = ui.doubleSpinBoxAngle3->value();
	gp.azimuth_4 = ui.doubleSpinBoxAngle4->value();
	gp.azimuth_5 = ui.doubleSpinBoxAngle5->value();
	gp.azimuth_6 = ui.doubleSpinBoxAngle6->value();
	return gp;
}

void WdgtCameraParameters::toGui(const GuiParams& p)
{
	ui.horizontalSliderFOV->setValue(convertParameterToGui(p.fov, 45.0f, 85.0f));
	ui.horizontalSliderOmniFOV->setValue(convertParameterToGui(p.omni_fov, 45.0f, 85.0f));
	ui.horizontalSliderKappa1->setValue(convertParameterToGui(p.kappa_1, 0.0f, 0.003f));
	ui.horizontalSliderKappa2->setValue(convertParameterToGui(p.kappa_2, 0.0f, 0.0003f));
	ui.horizontalSliderOmniAngle->setValue(convertParameterToGui(p.omni_altitude, 0.0f, 60.0f));
	ui.horizontalSliderOmniDist->setValue(convertParameterToGui(p.omni_dist, 0.03f, 0.10f));
	ui.horizontalSliderAngleTop->setValue(convertParameterToGui(p.azimuth_top, 0.0f, 360.0f));
	ui.doubleSpinBoxAngle1->setValue(p.azimuth_1);
	ui.doubleSpinBoxAngle2->setValue(p.azimuth_2);
	ui.doubleSpinBoxAngle3->setValue(p.azimuth_3);
	ui.doubleSpinBoxAngle4->setValue(p.azimuth_4);
	ui.doubleSpinBoxAngle5->setValue(p.azimuth_5);
	ui.doubleSpinBoxAngle6->setValue(p.azimuth_6);
	dirty = true;
}

void WdgtCameraParameters::save(const std::string& filename)
{
	GuiParams gp = fromGui();

	using boost::property_tree::ptree;
	ptree pt;
	pt.put("fov", gp.fov);
	pt.put("omni.fov", gp.omni_fov);
	pt.put("kappa_1", gp.kappa_1);
	pt.put("kappa_2", gp.kappa_2);
	pt.put("omni.altitude", gp.omni_altitude);
	pt.put("omni.dist", gp.omni_dist);
	pt.put("omni.azimuth_top", gp.azimuth_top);
	pt.put("omni.azimuth_1", gp.azimuth_1);
	pt.put("omni.azimuth_2", gp.azimuth_2);
	pt.put("omni.azimuth_3", gp.azimuth_3);
	pt.put("omni.azimuth_4", gp.azimuth_4);
	pt.put("omni.azimuth_5", gp.azimuth_5);
	pt.put("omni.azimuth_6", gp.azimuth_6);
	write_xml(filename, pt);
}

void WdgtCameraParameters::load(const std::string& filename)
{
	using boost::property_tree::ptree;
	ptree pt;
	read_xml(filename, pt);
	GuiParams gp;
	gp.fov = pt.get<float>("fov");
	gp.omni_fov = pt.get<float>("omni.fov");
	gp.kappa_1 = pt.get<float>("kappa_1");
	gp.kappa_2 = pt.get<float>("kappa_2");
	gp.omni_altitude = pt.get<float>("omni.altitude");
	gp.omni_dist = pt.get<float>("omni.dist");
	gp.azimuth_top = pt.get<float>("omni.azimuth_top");
	gp.azimuth_1 = pt.get<float>("omni.azimuth_1");
	gp.azimuth_2 = pt.get<float>("omni.azimuth_2");
	gp.azimuth_3 = pt.get<float>("omni.azimuth_3");
	gp.azimuth_4 = pt.get<float>("omni.azimuth_4");
	gp.azimuth_5 = pt.get<float>("omni.azimuth_5");
	gp.azimuth_6 = pt.get<float>("omni.azimuth_6");

	toGui(gp);
}

void WdgtCameraParameters::onSave()
{
	save("camera.cfg");
}

void WdgtCameraParameters::onLoad()
{
	load("camera.cfg");
}

void WdgtCameraParameters::valueChanged()
{
	std::vector<Edvs::EdvsSensorModelF>& cp = params_;
	constexpr unsigned int TOP_ID = OMNIROB_CNT-1;

	GuiParams gp = fromGui();

	// common
	for(unsigned int i=0; i<cp.size(); i++) {
		cp[i].position = Eigen::Vector3f::Zero();
		cp[i].setOpeningAngleDeg(gp.omni_fov);
		cp[i].kappa_1 = gp.kappa_1;
		cp[i].kappa_2 = gp.kappa_2;
	}
	// sides
	const float alpha = gp.omni_altitude / 180.0f * 3.1415f;
	const float alpha_sin = std::sin(alpha);
	const float alpha_cos = std::cos(alpha);
	const float theta[OMNIROB_CNT-1] = {
		gp.azimuth_1,
		gp.azimuth_2 / 180.0f * 3.1415f,
		gp.azimuth_3 / 180.0f * 3.1415f,
		gp.azimuth_4 / 180.0f * 3.1415f,
		gp.azimuth_5 / 180.0f * 3.1415f,
		gp.azimuth_6 / 180.0f * 3.1415f
	};
	for(unsigned int i=0; i<OMNIROB_CNT-1; i++) {
		cp[i].rotation = Edvs::Omnirob::FindRotationBase(
			Edvs::EdvsSensorModelF::RotateZ(theta[i], {0,alpha_cos,alpha_sin}),
			Edvs::EdvsSensorModelF::RotateZ(theta[i], {-1,0,0})
		);
		cp[i].position = gp.omni_dist * cp[i].rotation.col(2);
	}
	// top
	const float theta0 = gp.azimuth_top / 180.0f * 3.1415f;
	cp[TOP_ID].setOpeningAngleDeg(gp.fov);
	cp[TOP_ID].rotation = Edvs::Omnirob::FindRotationBase(
		{0,0,1},
		Edvs::EdvsSensorModelF::RotateZ(theta0, {-1,0,0})
	);
	cp[TOP_ID].position = gp.omni_dist * cp[TOP_ID].rotation.col(2);

	dirty = true;
}
