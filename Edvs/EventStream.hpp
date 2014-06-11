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

			std::vector<edvs_event_t> read() const
			{
				if(!is_open()) {
					return {};
				}
				else {
					return impl_->pop_events();
				}
			}

			// void read(std::vector<edvs_event_t>& v, std::vector<edvs_special_t>& v_special) const {
			// 	if(!is_open()) {
			// 		v.clear();
			// 		v_special.clear();
			// 	}
			// 	else {
			// 		size_t v_special_n = v_special.size();
			// 		ssize_t m = edvs_read_ext(impl_->h, v.data(), v.size(), v_special.data(), &v_special_n);
			// 		if(m >= 0) {
			// 			v.resize(m);
			// 		}
			// 		else {
			// 			v.clear();
			// 		}
			// 		// special
			// 		if(v_special_n >= 0) {
			// 			v_special.resize(v_special_n);
			// 		}
			// 		else {
			// 			v_special.clear();
			// 		}
			// 	}
			// }

			void write(const std::string& cmd) const {
				std::string cmdn = cmd;
				cmdn += '\n';
				edvs_write(impl_->handle(), (char*)cmdn.data(), cmdn.length());
			}

		private:
			mutable std::shared_ptr<impl::Handle> impl_;
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

		std::vector<edvs_event_t> read() const
		{
			std::vector<edvs_event_t> v;
			uint8_t id = 0;
			// capture events
			for(const auto& s : streams_) {
				// read maximum number of events from stream
				auto tmp = s.read();
				// set correct ID
				for(auto& e : tmp) {
					e.id = id;
				}
				// add events
				v.insert(v.end(), tmp.begin(), tmp.end());
				// std::cout << (int)id << " " << v.size() << std::endl;
				id ++;
			}
			// sort events into correct order
			std::sort(v.begin(), v.end(),
				[](const edvs_event_t& a, const edvs_event_t& b) {
					return a.t < b.t;
				});
			return v;
		}

		// void read(std::vector<edvs_event_t>& v, std::vector<edvs_special_t>& v_special) const {
		// 	if(streams_.size() == 0) {
		// 		return;
		// 	}
		// 	else if(streams_.size() == 1) {
		// 		streams_.front().read(v, v_special);
		// 	}
		// 	else {
		// 		// NOT IMPLEMENTED
		// 	}
		// }

		void write(const std::string& cmd) const {
			for(const auto& s : streams_) {
				s.write(cmd);
			}
		}

	private:
		std::vector<impl::SingleEventStream> streams_;
	};

}

#endif
