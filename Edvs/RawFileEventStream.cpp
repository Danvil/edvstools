#include "RawFileEventStream.hpp"
#include "LoadSaveEvents.hpp"

namespace Edvs
{

RawFileEventStream::RawFileEventStream(const std::string& filename, uint32_t fixed_dt)
: fixed_dt_(fixed_dt)
{
	events_ = LoadRawEvents(filename);
	if(fixed_dt_ == 0) {
		last_time_ = std::chrono::high_resolution_clock::now();
	}
	last_event_ = events_.begin() + 10; // HACK: ignore first 10 events - it's often junk
	if(last_event_ > events_.end()) {
		last_event_ = events_.end();
	}
	last_event_time_ = last_event_->time;
	dt_actual = 0;
}

// 	time_t prev_time = time - dt;
// //	std::cout << "time=" << time << ", dt=" << dt << ", prev_time" << prev_time << std::endl;
// 	auto last = std::lower_bound(events_.begin(), events_.end(), prev_time, [](const Event& x, time_t q) {
// 		return x.time < q;
// 	});
// //	auto last = std::find_if(events_.begin(), events_.end(), [prev_time](const Event& e) { return e.time > prev_time; });
// 	if(last == events_.end()) {
// 		return;
// 	}
// 	auto next = std::find_if(last, events_.end(), [time](const Event& e) { return e.time > time; });
// //	std::cout << "Events: " << std::distance(last, next) << std::endl;


void RawFileEventStream::read(std::vector<RawEvent>& events)
{
	// use a clock to find out how many events should be returned
	int32_t dt_required;
	if(fixed_dt_ == 0) {
		auto time_now = std::chrono::high_resolution_clock::now();
		dt_required = std::chrono::duration_cast<std::chrono::microseconds>(time_now - last_time_).count();
		last_time_ = time_now;
	}
	else {
		dt_required = fixed_dt_;
	}
	// find next event
	auto next = last_event_;
	for(; next != events_.end(); ) {
		uint32_t t = next->time;
		uint32_t dt = 0;
		if(t < last_event_time_) {
			dt = 2*t; // HACK: we do not know the total wrap...
			                  //       ignored pt until end so add 0 til t twice
			                  //       this averages out the error
		}
		else {
			dt = (t - last_event_time_);
		}
		if(dt_actual+dt >= dt_required) {
			dt_actual -= dt_required;
			break;
		}
		dt_actual += dt;
		last_event_time_ = next->time;
		++next;
	}
	// forward events
	events.clear();
	events.insert(events.begin(), last_event_, next);
	last_event_ = next;
}

bool RawFileEventStream::eof() const
{
	return last_event_ == events_.end();
}

}
