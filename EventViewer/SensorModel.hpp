/*
 * SensorModel.hpp
 *
 *  Created on: Jun 18, 2012
 *      Author: david
 */

#ifndef SENSORMODEL_HPP_
#define SENSORMODEL_HPP_

#include "PixelViewCone.hpp"
#include "Pose.hpp"
#include <Eigen/Dense>

namespace Edvs
{

template<typename K>
struct EdvsSensorModel
{
	typedef Eigen::Matrix<K,3,1> vec3_t;
	typedef Eigen::Matrix<K,2,1> vec2_t;
	typedef Eigen::Matrix<K,3,3> mat3_t;

	static constexpr int RETINA_SIZE = 128; // size of retina

public:
	// optical center x
	K center_x;

	// optical center y
	K center_y;

	// first parameter of lens distortion
	K kappa_1;

	// second parameter of lens distortion
	K kappa_2;

	// sensor position
	vec3_t position;

	// sensor orientation matrix
	mat3_t rotation;

private:
	K opening_angle_;
	K retina_proj_param_;

public:
	EdvsSensorModel() {
		center_x = 0.5f * static_cast<K>(RETINA_SIZE);
		center_y = 0.5f * static_cast<K>(RETINA_SIZE);
		kappa_1 = 0.0;
		kappa_2 = 0.0;
		position = vec3_t::Zero();
		rotation = mat3_t::Identity();
		setOpeningAngleDeg(53.0f);
	}

	void setOpeningAngleDeg(K alpha_deg) {
		opening_angle_ = alpha_deg * K(M_PI / 180.0);
		retina_proj_param_ = K(0.5) * static_cast<K>(RETINA_SIZE) / std::tan(K(0.5) * opening_angle_);
	}

	float getOpeningAngle() const {
		return opening_angle_;
	}

	float getPixelOpeningAngle() const {
		return opening_angle_ / 128.0f;
	}

	K computePixelOpeningAngleWithUncertainty(K x, K y) const {
		// constexpr K MAGIC = 3.0f*3.1415/180.0f / 64.0f;
		// x -= center_x;
		// y -= center_y;
		// return 3.0f*getPixelOpeningAngle() + MAGIC*std::sqrt(x*x + y*y);
		return getPixelOpeningAngle() + 2.5f/180.0f*3.1415;
	}

	double getPixelFocalLength() const {
		return retina_proj_param_;
	}

	/** Undistort */
	void undistortCentered(K& x, K& y) const {
		const K r2 = x*x + y*y;
		const K r = std::sqrt(r2);
		const K L = 1 + kappa_1*r + kappa_2*r2;
		x *= L;
		y *= L;
	}

	/** Transforms a pixel on the retina to a ray in world coordinates
	 * @param undistort whether to compensates lens distortion
	 */
	vec3_t computeEventDirection(K x, K y) const {
		// compensate center
		x -= center_x;
		y -= center_y;
		// compensate distortion
		undistortCentered(x, y);
		// compute camera coordinates
		return vec3_t{x, -y, retina_proj_param_}.normalized();
	}

	/** Computes pixel view cone for an event */
	PixelViewCone createPixelViewCone(K ex, K ey) const {
		return PixelViewCone {
			position,
			rotation * computeEventDirection(ex, ey),
			computePixelOpeningAngleWithUncertainty(ex,ey),
			1.0f
		};
	}

	/** Transforms a point from world to camera coordinates */
	vec3_t TransformWorldToCamera(const vec3_t& x_world, const vec3_t& camera_pos, K camera_rot_sin, K camera_rot_cos ) {
		return RotateZ(-camera_rot_sin, camera_rot_cos, x_world - camera_pos);
	}

	/** Checks if a point in camera coordinates is visible by the retina */
	bool isVisible(const vec3_t& pos) const {
		const int ix = static_cast<int>(pos.x() / pos.z() * retina_proj_param_);
		const int iy = static_cast<int>(pos.y() / pos.z() * retina_proj_param_);
		return -RETINA_SIZE/2 <= ix && ix <= RETINA_SIZE/2
			&& -RETINA_SIZE/2 <= iy && iy <= RETINA_SIZE/2;
	}

	vec2_t projectCameraOnRetina(const vec3_t& x_cam) const {
		const float q = retina_proj_param_ / x_cam.z();
		return q * vec2_t(x_cam.x(), x_cam.y());
	}

	/** Checks if a point in retina coordinates is visible */
	bool isVisible(const vec2_t& r) const {
		const int ix = static_cast<int>(r.x());
		const int iy = static_cast<int>(r.y());
		return -RETINA_SIZE/2 <= ix && ix <= RETINA_SIZE/2
			&& -RETINA_SIZE/2 <= iy && iy <= RETINA_SIZE/2;
	}

	/** Transforms a point from camera to world coordinates */
	vec3_t TransformCameraToWorld(const vec3_t& x_camera, const vec3_t& camera_pos, K camera_rot_sin, K camera_rot_cos ) {
		return camera_pos + RotateZ(camera_rot_sin, camera_rot_cos, x_camera);
	}

	static vec3_t RotateZ(K rot_sin, K rot_cos, const vec3_t& v) {
		return vec3_t {
			rot_cos*v.x() - rot_sin*v.y(),
			rot_sin*v.x() + rot_cos*v.y(),
			v.z()
		};
	}

	static vec3_t RotateZ(K angle, const vec3_t& v) {
		return RotateZ(std::sin(angle), std::cos(angle), v);
	}

	static PixelViewCone Transform(const vec3_t& pos, K rot_sin, K rot_cos, const PixelViewCone& pvc) {
		return PixelViewCone {
			pos + pvc.ray_start,
			RotateZ(rot_sin, rot_cos, pvc.ray_dir),
			pvc.opening_angle,
			pvc.weight
		};
	}

};

typedef EdvsSensorModel<float> EdvsSensorModelF;

}

#endif
