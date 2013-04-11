#include "EventCapture.hpp"

namespace Edvs
{
	EventCapture::EventCapture(const EventStreamHandle& stream, EventCallbackType callback)
	: stream_(stream), cb_(callback)
	{
		running_ = true;
		thread_ = boost::thread(&EventCapture::threadMain, this);
	}

	EventCapture::~EventCapture()
	{
		running_ = false;
		thread_.join();
	}

	void EventCapture::threadMain()
	{
		std::vector<RawEvent> buffer;
		while(running_) {
			stream_->read(buffer);
			cb_(buffer);
		}
	}

	EventCaptureHandle RunEventCapture(const EventStreamHandle& stream, EventCallbackType callback)
	{
		return EventCaptureHandle(new EventCapture(stream, callback));
	}
}
