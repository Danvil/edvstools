#ifndef INCLUDE_EDVS_RAWEVENTTRANSLATOR_HPP
#define INCLUDE_EDVS_RAWEVENTTRANSLATOR_HPP

#include "EventStream.hpp"
#include <vector>
#include <chrono>

namespace Edvs
{

	class RawEventTranslator
	: public EventStream
	{
	public:
		RawEventTranslator(const RawEventStreamHandle& raw_stream, uint32_t id=0);
		
		~RawEventTranslator();
		
		void read(std::vector<Event>& events);

		bool eof() const {
			return raw_stream_->eof();
		}

	private:
		RawEventStreamHandle raw_stream_;
		std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
		uint32_t id_;
	};

}

#endif
