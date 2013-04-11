#ifndef INCLUDE_EDVS_EVENTSTREAM_HPP
#define INCLUDE_EDVS_EVENTSTREAM_HPP

#include "Event.hpp"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace Edvs
{

	class EventStream
	{
	public:
		virtual ~EventStream() {}

		virtual void read(std::vector<RawEvent>& events) = 0;

	};

	typedef boost::shared_ptr<EventStream> EventStreamHandle;

}

#endif
