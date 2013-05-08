#ifndef INCLUDED_EDVS_TOOLS_MAP_PIXELVIEWCONE_HPP
#define INCLUDED_EDVS_TOOLS_MAP_PIXELVIEWCONE_HPP

#include <Eigen/Dense>

namespace Edvs {

/** Definition of a pixel view cone */
struct PixelViewCone
{
	typedef Eigen::Vector3f vec3_t;

	// start position of the pixel view cone
	vec3_t ray_start;

	// direction of the pixel view cone
	vec3_t ray_dir;

	// opening angle of the pixel view cone
	float opening_angle;

	// weight of the cone
	float weight;

};

}

#endif
