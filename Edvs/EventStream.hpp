#ifndef INCLUDE_EDVS_EVENTSTREAM_HPP
#define INCLUDE_EDVS_EVENTSTREAM_HPP

#include "Event.hpp"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace Edvs
{

	template<typename event_t>
	class IEventStream
	{
	public:
		virtual ~IEventStream() {}

		virtual void read(std::vector<event_t>& events) = 0;

		virtual bool eof() const = 0;

	};

	typedef IEventStream<RawEvent> RawEventStream;
	typedef boost::shared_ptr<RawEventStream> RawEventStreamHandle;

	typedef IEventStream<Event> EventStream;
	typedef boost::shared_ptr<EventStream> EventStreamHandle;

}

#endif
