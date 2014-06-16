#include "EventStream.hpp"
#include "edvs.h"
#include "Event.hpp"
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>

namespace Edvs
{
	class SingleEventStream : public IEventStream
	{
	public:
		SingleEventStream() {}

		SingleEventStream(const SingleEventStream&) = delete;
		SingleEventStream& operator=(const SingleEventStream&) = delete;
		
		SingleEventStream(const std::string& uri)
		{
			open(uri);
			run();
		}

		~SingleEventStream()
		{
			close();
		}
		
		void open(const std::string& uri)
		{
			h = edvs_open(uri.c_str());
		}

		void close()
		{
			if(is_open()) {
				if(is_running_) {
					is_running_ = false;
					thread_.join();
				}
				if(h) {
					edvs_close(h);
				}
			}
		}

		void run()
		{
			last_time_ = 0;
			edvs_run(h);
			is_running_ = true;
			thread_ = std::thread(&SingleEventStream::runImpl, this);
		}

		bool is_open() const
		{
			return h;
		}

		bool is_master() const
		{
			return edvs_get_master_slave_mode(h) == 1;
		}

		bool is_slave() const
		{
			return edvs_get_master_slave_mode(h) != 1;
		}

		bool eos() const
		{
			std::lock_guard<std::mutex> lock(mtx_);
			return is_open() && edvs_eos(h) && events_.empty();
		}

		std::vector<edvs_event_t> read()
		{
			if(!is_open()) {
				return {};
			}
			else {
				std::vector<edvs_event_t> v = pop_events();
				if(!v.empty()) {
					last_time_ = v.back().t;
				}
				return v;
			}
		}

		uint64_t last_timestamp() const
		{ return last_time_; }

		void write(const std::string& cmd) const
		{
			std::string cmdn = cmd;
			cmdn += '\n';
			edvs_write(h, (char*)cmdn.data(), cmdn.length());
		}

	private:
		std::vector<edvs_event_t> pop_events()
		{
			std::lock_guard<std::mutex> lock(mtx_);
			std::vector<edvs_event_t> tmp = std::move(events_); // TODO is this move correct?
			events_ = {};
			return tmp;
		}
		
		void runImpl()
		{
			std::vector<edvs_event_t> v;
			while(is_running_ && !edvs_eos(h)) {
				v.resize(1024);
				ssize_t m = edvs_read_ext(h, v.data(), v.size(), 0, 0);
				if(m >= 0) {
					v.resize(m);
				}
				else {
					v.clear();
				}
				std::lock_guard<std::mutex> lock(mtx_);
				events_.insert(events_.end(), v.begin(), v.end());
			}
		}
		
	private:
		bool is_running_;
		std::thread thread_;
		mutable std::mutex mtx_;
		std::vector<edvs_event_t> events_;
		edvs_stream_handle h;
		uint64_t last_time_;
	};


	class MultiEventStream : public IEventStream
	{
	public:
		MultiEventStream() {}
		
		MultiEventStream(const MultiEventStream&) = delete;
		MultiEventStream& operator=(const MultiEventStream&) = delete;
		
		MultiEventStream(const std::vector<std::string>& uris)
		{
			open(uris);
			run();
		}

		unsigned num_streams() const
		{
			return streams_.size();
		}

		void open(const std::vector<std::string>& uris)
		{
			for(const std::string& uri : uris) {
				SingleEventStream* ses = new SingleEventStream();
				ses->open(uri);
				streams_.emplace_back(ses);
			}
		}

		void run()
		{
			// master first
			for(auto& s : streams_) {
				if(s->is_master())
					s->run();
			}
			// then slaves
			for(auto& s : streams_) {
				if(s->is_slave())
					s->run();
			}
		}

		bool is_open() const
		{
			if(streams_.size() == 0) {
				return false;
			}
			for(const auto& s : streams_) {
				if(!s->is_open()) {
					return false;
				}
			}
			return true;
		}

		bool eos() const
		{
			// no stream => eos
			if(streams_.size() == 0) {
				return true;
			}
			// still events in queue => not eos
			if(!events_.empty()) {
				return false;
			}
			// check if any stream is eos
			for(const auto& s : streams_) {
				if(s->eos()) {
					return true;
				}
			}
			return false;
		}

		void close()
		{
			for(const auto& s : streams_) {
				s->close();
			}
		}

		std::vector<edvs_event_t> read()
		{
			// compute the "common time"
			// this is the time up to which all streams have delivered events
			uint64_t common_time = std::numeric_limits<uint64_t>::max();
			// id of stream is its position in the list of streams 
			uint8_t id = 0;
			// iterate over all streams and get events
			bool all_empty = true;
			for(const auto& s : streams_) {
				// read maximum number of events from stream
				auto tmp = s->read();
				// update common time
				uint64_t lastts = s->last_timestamp();
				common_time = std::min(common_time, lastts);
				// check if streams return nothing
				all_empty = all_empty && tmp.empty();
				// set correct ID
				// we only do this for multiple streams
				// FIXME need a mechanism to check if we need to do it ...
				if(streams_.size() > 1) {
					for(auto& e : tmp) {
						e.id = id;
	//					std::cout << (int)(e.id) << " " << e.t << std::endl;
					}
				}
				// add events to our buffer
				events_.insert(events_.end(), tmp.begin(), tmp.end());
				// next stream gets next id
				id ++;
			}
			if(all_empty) {
				return {};
			}
			// sort our buffer by timestamps
			std::sort(events_.begin(), events_.end(),
				[](const edvs_event_t& a, const edvs_event_t& b) {
					return a.t < b.t;
				});
			// find location of common time in our buffer
			auto it = std::upper_bound(events_.begin(), events_.end(), common_time,
				[](uint64_t t, const edvs_event_t& e) { return t < e.t; });
			// remove the corresponding chunk of events from our buffer and return them
			std::vector<edvs_event_t> ret(events_.begin(), it);
			events_.erase(events_.begin(), it);
			return ret;
		}

		void write(const std::string& cmd) const
		{
			for(const auto& s : streams_) {
				s->write(cmd);
			}
		}

	private:
		std::vector<std::unique_ptr<SingleEventStream>> streams_;
		std::vector<edvs_event_t> events_;
	};

	std::shared_ptr<IEventStream> OpenEventStream(const std::string& uri)
	{
		return std::make_shared<SingleEventStream>(uri);
	}

	std::shared_ptr<IEventStream> OpenEventStream(const std::initializer_list<std::string>& uris)
	{
		return std::make_shared<MultiEventStream>(uris);
	}

	std::shared_ptr<IEventStream> OpenEventStream(const std::vector<std::string>& uris)
	{
		return std::make_shared<MultiEventStream>(uris);
	}

}
