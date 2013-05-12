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
		typedef boost::function<void(const std::vector<Event>&)> callback_event_t;
		typedef boost::function<void(const std::vector<edvs_special_t>&)> callback_special_t;

	private:
		struct Impl
		{
			Impl(const EventStream& stream, callback_event_t callback_event, callback_special_t callback_special) {
				is_running_ = true;
				thread_ = boost::thread(&Impl::threadMain, this, stream, callback_event, callback_special);
			}
			~Impl() {
				is_running_ = false;
				thread_.join();
			}
			void threadMain(const EventStream& stream, callback_event_t callback_event, callback_special_t callback_special) {
				std::vector<edvs_event_t> buffer;
				std::vector<edvs_special_t> buffer_special;
				while(is_running_ && !stream.eos()) {
					buffer.resize(1024);
					buffer_special.resize(128);
					stream.read(buffer, buffer_special);
					callback_event(buffer);
					callback_special(buffer_special);
				}
			}
			bool is_running_;
			boost::thread thread_;
		};

	public:
		EventCapture() {}

		EventCapture(const EventStream& stream, callback_event_t callback_event) {
			start(stream, callback_event, callback_special_t());
		}

		EventCapture(const EventStream& stream, callback_event_t callback_event, callback_special_t callback_special) {
			start(stream, callback_event, callback_special);
		}

		void start(const EventStream& stream, callback_event_t callback_event, callback_special_t callback_special) {
			impl_ = boost::make_shared<Impl>(stream, callback_event, callback_special);
		}

		bool is_running() const {
			return impl_->is_running_;
		}

		void stop() {
			impl_.reset();
		}

	private:
		boost::shared_ptr<Impl> impl_;
	};

}

#endif
