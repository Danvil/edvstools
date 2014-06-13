#ifndef INCLUDE_EDVS_EVENTSTREAM_HPP
#define INCLUDE_EDVS_EVENTSTREAM_HPP

#include "edvs.h"
#include "Event.hpp"
#include <boost/thread.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>

namespace Edvs
{

	namespace impl
	{
		struct Handle
		{
			Handle(const std::string& uri)
			{
				h = edvs_open(uri.c_str());
			}
			
			~Handle()
			{
				is_running_ = false;
				thread_.join();
				if(h) {
					edvs_close(h);
				}
			}
			
			const edvs_stream_handle& handle() const
			{ return h; }

			void run()
			{
				edvs_run(h);
				is_running_ = true;
				thread_ = boost::thread(&Handle::runImpl, this);
			}

			std::vector<edvs_event_t> pop_events()
			{
				std::vector<edvs_event_t> tmp;
				{
					boost::interprocess::scoped_lock<boost::mutex> lock(mtx_);
					tmp = std::move(events_);
					events_ = {};
				}
				return tmp;
			}
			
		private:
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
					boost::interprocess::scoped_lock<boost::mutex> lock(mtx_);
					events_.insert(events_.end(), v.begin(), v.end());
				}
			}
			
		private:
			bool is_running_;
			boost::thread thread_;
			boost::mutex mtx_;
			std::vector<edvs_event_t> events_;
			edvs_stream_handle h;
		};

		class SingleEventStream
		{
		public:
			SingleEventStream() {}
			
			SingleEventStream(const std::string& uri) {
				open(uri);
				run();
			}

			void open(const std::string& uri) {
				impl_ = std::make_shared<impl::Handle>(uri);
			}

			void run() {
				last_time_ = 0;
				impl_->run();
			}

			bool is_open() const {
				return impl_ && impl_->handle();
			}

			bool is_master() const {
				return edvs_get_master_slave_mode(impl_->handle()) == 1;
			}

			bool is_slave() const {
				return edvs_get_master_slave_mode(impl_->handle()) != 1;
			}

			bool eos() const {
				return !impl_ || edvs_eos(impl_->handle());
			}

			void close() {
				impl_.reset();
			}

			std::vector<edvs_event_t> read()
			{
				if(!is_open()) {
					return {};
				}
				else {
					std::vector<edvs_event_t> v = impl_->pop_events();
					if(!v.empty()) {
						last_time_ = v.back().t;
					}
					return v;
				}
			}

			uint64_t last_timestamp() const
			{ return last_time_; }

			void write(const std::string& cmd) const {
				std::string cmdn = cmd;
				cmdn += '\n';
				edvs_write(impl_->handle(), (char*)cmdn.data(), cmdn.length());
			}

		private:
			mutable std::shared_ptr<impl::Handle> impl_;
			mutable uint64_t last_time_;
		};
	}

	class EventStream
	{
	public:
		EventStream() {}
		
		EventStream(const std::string& uri) {
			open({uri});
			run();
		}

		EventStream(const std::vector<std::string>& uris) {
			open(uris);
			run();
		}

		unsigned num_streams() const {
			return streams_.size();
		}

		void open(const std::vector<std::string>& uris) {
			for(const std::string& uri : uris) {
				impl::SingleEventStream ses;
				ses.open(uri);
				streams_.push_back(ses);
			}
		}

		void run() {
			// master first
			for(auto& s : streams_) {
				if(s.is_master())
					s.run();
			}
			// then slaves
			for(auto& s : streams_) {
				if(s.is_slave())
					s.run();
			}
		}

		bool is_open() const {
			if(streams_.size() == 0) {
				return false;
			}
			for(const auto& s : streams_) {
				if(!s.is_open()) {
					return false;
				}
			}
			return true;
		}

		bool eos() const {
			if(streams_.size() == 0) {
				return false;
			}
			for(const auto& s : streams_) {
				if(s.eos()) {
					return true;
				}
			}
			return false;
		}

		void close() {
			for(auto& s : streams_) {
				s.close();
			}
		}

		std::vector<edvs_event_t> read()
		{
			uint8_t id = 0;
			// capture events
			uint64_t common_time = std::numeric_limits<uint64_t>::max();
			for(auto& s : streams_) {
				// read maximum number of events from stream
				auto tmp = s.read();
				// update common time
				common_time = std::min(common_time, s.last_timestamp());
				// set correct ID
				for(auto& e : tmp) {
					e.id = id;
				}
				// add events
				events_.insert(events_.end(), tmp.begin(), tmp.end());
				// std::cout << (int)id << " " << v.size() << std::endl;
				id ++;
			}
			// sort events into correct order
			std::sort(events_.begin(), events_.end(),
				[](const edvs_event_t& a, const edvs_event_t& b) {
					return a.t < b.t;
				});
			// return everything up to common time
			auto it = std::upper_bound(events_.begin(), events_.end(), common_time,
				[](uint64_t t, const edvs_event_t& e) { return t < e.t; });
			std::vector<edvs_event_t> ret(events_.begin(), it);
			events_.erase(events_.begin(), it);
			return ret;
		}

		void write(const std::string& cmd) const {
			for(const auto& s : streams_) {
				s.write(cmd);
			}
		}

	private:
		std::vector<impl::SingleEventStream> streams_;
		std::vector<edvs_event_t> events_;
	};

}

#endif
