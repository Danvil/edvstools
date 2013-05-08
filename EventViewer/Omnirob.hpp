#ifndef EDVS_OMNIROB_HPP
#define EDVS_OMNIROB_HPP

#include <Edvs/Event.hpp>
#include "SensorModel.hpp"
#include <cassert>

namespace Edvs
{
	namespace Omnirob
	{

		inline
		Eigen::Matrix3f FindRotationBase(const Eigen::Vector3f& dir, const Eigen::Vector3f& x) {
			Eigen::Matrix3f base;
			base.col(0) = x;
			base.col(1) = dir.cross(x);
			base.col(2) = dir;
			return base;
		}

		class EventMapper
		{
		public:
			void setCamCount(unsigned int n) {
				cam_params_.resize(n);
			}

			void setCameraParameters(const std::vector<EdvsSensorModelF>& v_cp) {
				cam_params_ = v_cp;
			}

			const std::vector<EdvsSensorModelF>& getCameraParameters() const {
				return cam_params_;
			}

			PixelViewCone map(const Event& event) const {
				assert(event.id < cam_params_.size());
				return cam_params_[event.id].createPixelViewCone(event.x, event.y);
			}

		private:
			std::vector<EdvsSensorModelF> cam_params_;
		};

		/** Stereographic projection from (0,0,-1) */
		inline
		Eigen::Vector2f StereographicProjection(const Eigen::Vector3f& v) {
			const float q = 1.0f / (v.norm() + v.z());
			return { q*v.x(), q*v.y() };
		}

	}

}

#endif
