#include "FileEventStream.hpp"
#include "LoadSaveEvents.hpp"

namespace Edvs
{

FileEventStream::FileEventStream(const std::string& filename, uint32_t fixed_dt)
: fixed_dt_(fixed_dt)
{
	events_ = LoadRawEvents(filename);
	last_event_ = events_.begin();
	if(fixed_dt_ == 0) {
		last_time_ = std::chrono::high_resolution_clock::now();
	}
}

void FileEventStream::read(std::vector<RawEvent>& events)
{
	// use a clock to find out how many events should be returned
	uint32_t dt;
	if(fixed_dt_ == 0) {
		dt = fixed_dt_;
	}
	else {
		auto time_now = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::microseconds>(time_now - last_time_).count();
		last_time_ = time_now;
	}
	// find next event
	auto next = last_event_;
	uint32_t pt = last_event_->time;
	uint32_t dt_actual = 0;
	for(; next!=events_.end(); ++next) {
		uint32_t t = next->time;
		if(t < pt) {
			dt_actual += 2*t; // HACK: we do not know the total wrap...
			                  //       ignored pt until end so add 0 til t twice
			                  //       this averages out the error
		}
		else {
			dt_actual += (t - pt);
		}
		pt = t;
		if(dt_actual >= dt) {
			break;
		}
	}
	// forward events
	events.clear();
	events.insert(events.begin(), last_event_, next);
	last_event_ = next;
}

}
