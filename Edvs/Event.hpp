#ifndef INCLUDE_EDVS_EVENT_HPP_
#define INCLUDE_EDVS_EVENT_HPP_

#include <stdint.h>
#include <iostream>

namespace Edvs
{
	/** An eDVS event */
	struct RawEvent
	{
		// timestamp
		uint32_t time;

		// event pixel coordinate
		unsigned char x, y;

		// parity
		bool parity;

		friend inline std::ostream& operator<<(std::ostream& os, const Edvs::RawEvent& e) {
			os << "[t=" << e.time << ", p=" << e.parity << ", (" << static_cast<int>(e.x) << ", " << static_cast<int>(e.y) << ")]";
			return os;
		}

	};

	struct Event
	{
		// timestamp
		uint64_t time;

		// device id
		uint32_t id;

		// event pixel coordinate
		float x, y;

		// parity
		bool parity;

		friend inline std::ostream& operator<<(std::ostream& os, const Edvs::Event& e) {
			os << "[t=" << e.time << ", id=" << e.id << ", p=" << e.parity << ", (" << e.x << ", " << e.y << ")]";
			return os;
		}

	};

}

#endif
