#ifndef INCLUDED_EDVS_RAWFILEEVENTSTREAM_HPP
#define INCLUDED_EDVS_RAWFILEEVENTSTREAM_HPP

#include "EventStream.hpp"
#include <chrono>
#include <fstream>

namespace Edvs
{
	class RawFileEventStream
	: public RawEventStream
	{
	public:
		RawFileEventStream(const std::string& filename, uint32_t fixed_dt=0);
		
		void read(std::vector<RawEvent>& events);

		bool eof() const;

	private:
		std::vector<RawEvent> events_;
		uint32_t fixed_dt_;
		std::chrono::time_point<std::chrono::high_resolution_clock> last_time_;
		std::vector<RawEvent>::const_iterator last_event_;
		uint32_t last_event_time_;
		int32_t dt_actual;
	};
}

#endif
