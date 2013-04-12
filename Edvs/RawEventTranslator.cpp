#include "RawEventTranslator.hpp"
#include <iostream>

//#define VERBOSE

namespace Edvs
{
	RawEventTranslator::RawEventTranslator(const RawEventStreamHandle& raw_stream, uint32_t id)
	: raw_stream_(raw_stream)
	{
		start_time_ = std::chrono::high_resolution_clock::now();
	}

	RawEventTranslator::~RawEventTranslator()
	{
	}

	void RawEventTranslator::read(std::vector<Event>& events)
	{
		// read raw events
		std::vector<RawEvent> raw_events;
		raw_stream_->read(raw_events);
		// get current time
		auto time_now = std::chrono::high_resolution_clock::now();
		uint64_t dt = std::chrono::duration_cast<std::chrono::microseconds>(time_now - start_time_).count();
		// transform events
		events.resize(raw_events.size());
		std::transform(raw_events.begin(), raw_events.end(), events.begin(),
			[this,dt](const RawEvent& r) {
				Event e;
				e.time = dt; // FIXME additionally use raw event timestamp
				e.id = this->id_;
				e.x = static_cast<float>(r.x);
				e.y = static_cast<float>(r.y);
				e.parity = static_cast<float>(r.parity);
				return e;
			});
	}

}
