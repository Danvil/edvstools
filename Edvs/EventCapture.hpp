#ifndef INCLUDE_EDVS_EVENTCAPTURE_HPP_
#define INCLUDE_EDVS_EVENTCAPTURE_HPP_

#include "EventStream.hpp"
#include <boost/thread.hpp>
#include <functional>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <vector>

namespace Edvs
{
	template<typename event_t>
	struct TEventCapture
	{	
		typedef boost::shared_ptr<IEventStream<event_t>> handle_t;
		typedef std::function<void(const std::vector<event_t>&)> callback_t;

		TEventCapture(const handle_t& stream, callback_t callback)
		: stream_(stream), callback_(callback) {
			running_ = true;
			thread_ = boost::thread(&TEventCapture<event_t>::threadMain, this);
		}

		~TEventCapture() {
			running_ = false;
			thread_.join();
		}

	private:
		void threadMain() {
			while(running_) {
				stream_->read(buffer_);
				callback_(buffer_);
			}
		}

	private:
		handle_t stream_;
		callback_t callback_;
		bool running_;
		boost::thread thread_;
		std::vector<event_t> buffer_;
	};

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
	template<typename event_t>
	inline boost::shared_ptr<TEventCapture<event_t>> RunEventCapture(
		const typename TEventCapture<event_t>::handle_t& stream,
		typename TEventCapture<event_t>::callback_t callback
	) {
		return boost::make_shared<TEventCapture<event_t>>(stream, callback);
	}

	typedef boost::shared_ptr<TEventCapture<RawEvent>> RawEventCaptureHandle;
	typedef boost::shared_ptr<TEventCapture<Event>> EventCaptureHandle;

}

#endif
