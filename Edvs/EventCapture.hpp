#ifndef INCLUDE_EDVS_EVENTCAPTURE_HPP_
#define INCLUDE_EDVS_EVENTCAPTURE_HPP_

#include "EventStream.hpp"
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace Edvs
{
	typedef boost::function<void(const std::vector<RawEvent>&)> EventCallbackType;

	struct EventCapture
	{	
		EventCapture(const EventStreamHandle& stream, EventCallbackType callback);

		~EventCapture();

	private:
		void threadMain();

	private:
		EventStreamHandle stream_;
		EventCallbackType cb_;
		bool running_;
		boost::thread thread_;
	};

	typedef boost::shared_ptr<EventCapture> EventCaptureHandle;

	/** Runs event capture
	 * Capturing starts on object construction and ends on object destruction.
	 * Please assure that you now how shared_ptr works!
	 * Capturing runs in a separate thread and calls the callback
	 * function when new events are received.
	 * Please assure that the callback function is thread safe!
	 * Do not perform computational intensive operations in the callback function
	 * otherwise you will not be able to receive all events.
	 * @param device Edvs device handle
	 * @param callback Edvs callback function
	 * @param buffer_size Maximum bytes to read from the device at once
	 */
	EventCaptureHandle RunEventCapture(const EventStreamHandle& stream, EventCallbackType callback);

}

#endif
