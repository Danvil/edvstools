#ifndef INCLUDE_EDVS_EVENTCAPTURE_HPP_
#define INCLUDE_EDVS_EVENTCAPTURE_HPP_

#include "EventStream.hpp"
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <vector>

namespace Edvs
{
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
	struct EventCapture
	{
	public:
		typedef boost::function<void(const std::vector<Event>&)> callback_t;

	private:
		struct Impl
		{
			Impl(const EventStream& stream, callback_t callback) {
				running_ = true;
				thread_ = boost::thread(&Impl::threadMain, this, stream, callback);
			}
			~Impl() {
				running_ = false;
				thread_.join();
			}
		private:
			void threadMain(const EventStream& stream, callback_t callback) {
				std::vector<edvs_event_t> buffer(1024);
				while(running_) {
					buffer.resize(1024);
					stream.read(buffer);
					callback(buffer);
				}
			}
		private:
			bool running_;
			boost::thread thread_;
		};

	public:
		EventCapture() {}

		EventCapture(const EventStream& stream, callback_t callback) {
			start(stream, callback);
		}

		void start(const EventStream& stream, callback_t callback) {
			impl_ = boost::make_shared<Impl>(stream, callback);
		}

		void stop() {
			impl_.reset();
		}

	private:
		boost::shared_ptr<Impl> impl_;
	};

}

#endif
