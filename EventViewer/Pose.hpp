/*
 * Pose.hpp
 *
 *  Created on: Jun 29, 2012
 *      Author: david
 */

#ifndef EDVS_TOOLS_POSE_HPP_
#define EDVS_TOOLS_POSE_HPP_

#include <Eigen/Dense>

namespace Edvs
{

	/** Pose of a camera where the camera view axis is equal to the z axis */
	template<typename K>
	struct ZAlignedPose
	{
		typedef Eigen::Matrix<K,3,1> vec3_t;

		/** Position */
		vec3_t pos;

		/** Rotation around the z axis */
		K rot;

		static ZAlignedPose Zero() {
			return ZAlignedPose{vec3_t::Zero(), K(0)};
		}

		const K& px() const { return pos[0]; }
		K& px() { return pos[0]; }

		const K& py() const { return pos[1]; }
		K& py() { return pos[1]; }

		const K& pz() const { return pos[2]; }
		K& pz() { return pos[2]; }

		friend std::ostream& operator<<(std::ostream& os, const ZAlignedPose& x) {
			os << "[pos=(" << x.pos.transpose() << "), rot=" << x.rot << ")]";
			return os;
		}

	};

}

#endif
