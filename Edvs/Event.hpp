#ifndef INCLUDE_EDVS_EVENT_HPP_
#define INCLUDE_EDVS_EVENT_HPP_

#include "event.h"
#include <stdint.h>
#include <iostream>

namespace Edvs
{
	typedef edvs_event_t Event;
}

inline std::ostream& operator<<(std::ostream& os, const Edvs::Event& e)
{
	os << "[t=" << e.t
		<< ", id=" << e.id
		<< ", p=" << e.parity
		<< ", (" << e.x << ", " << e.y << ")]";
	return os;
}

namespace Edvs
{
	/** A normal edvs event with high-resolution timestamp, device id and real coordinates
	 * Structure size is 21 bytes
	 */
	struct EventF
	{
		// timestamp
		uint64_t time;

		// device id
		uint32_t id;

		// event pixel coordinate
		float x, y;

		// parity
		bool parity;

		friend inline std::ostream& operator<<(std::ostream& os, const Edvs::EventF& e) {
			os << "[t=" << e.time
				<< ", id=" << e.id
				<< ", p=" << e.parity
				<< ", (" << e.x << ", " << e.y << ")]";
			return os;
		}

	};

}

#endif
