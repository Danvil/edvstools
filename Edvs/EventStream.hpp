#ifndef INCLUDE_EDVS_EVENTSTREAM_HPP
#define INCLUDE_EDVS_EVENTSTREAM_HPP

#include "edvs.h"
#include "Event.hpp"
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
			Handle(const std::string& uri) {
				h = edvs_open(uri.c_str());
			}
			~Handle() {
				if(h) edvs_close(h);
			}
			edvs_stream_handle h;
		};

		class SingleEventStream
		{
		public:
			SingleEventStream() {}
			
			SingleEventStream(const std::string& uri) {
				open(uri);
			}

			void open(const std::string& uri) {
				impl_ = std::make_shared<impl::Handle>(uri);
			}

			bool is_open() const {
				return impl_ && impl_->h;
			}

			bool eos() const {
				return !impl_ || edvs_eos(impl_->h);
			}

			void close() {
				impl_.reset();
			}

			void read(std::vector<edvs_event_t>& v) const {
				if(!is_open()) {
					v.clear();
				}
				else {
					ssize_t m = edvs_read_ext(impl_->h, v.data(), v.size(), 0, 0);
					if(m >= 0) {
						v.resize(m);
					}
					else {
						v.clear();
					}
				}
			}

			void read(std::vector<edvs_event_t>& v, std::vector<edvs_special_t>& v_special) const {
				if(!is_open()) {
					v.clear();
					v_special.clear();
				}
				else {
					size_t v_special_n = v_special.size();
					ssize_t m = edvs_read_ext(impl_->h, v.data(), v.size(), v_special.data(), &v_special_n);
					if(m >= 0) {
						v.resize(m);
					}
					else {
						v.clear();
					}
					// special
					if(v_special_n >= 0) {
						v_special.resize(v_special_n);
					}
					else {
						v_special.clear();
					}
				}
			}

			std::vector<edvs_event_t> read() const {
				std::vector<edvs_event_t> v(1024); // FIXME how many can we read?
				std::vector<edvs_special_t> vs(128); // FIXME how many can we read?
				read(v, vs);
				return v;
			}

			void write(const std::string& cmd) const {
				std::string cmdn = cmd;
				cmdn += '\n';
				edvs_write(impl_->h, (char*)cmdn.data(), cmdn.length());
			}

		private:
			std::shared_ptr<impl::Handle> impl_;
		};
	}

	class EventStream
	{
	public:
		EventStream() {}
		
		EventStream(const std::string& uri) {
			open({uri});
		}

		EventStream(const std::vector<std::string>& uris) {
			open(uris);
		}

		unsigned num_streams() const {
			return streams_.size();
		}

		void open(const std::vector<std::string>& uris) {
			for(const std::string& uri : uris) {
				streams_.push_back(impl::SingleEventStream(uri));
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

		void read(std::vector<edvs_event_t>& v) const {
			if(streams_.size() == 0) {
				return;
			}
			else if(streams_.size() == 1) {
				streams_.front().read(v);
			}
			else {
				size_t num = v.size() / streams_.size();
				std::vector<edvs_event_t> tmp(num);
				v.clear();
				uint8_t id = 0;
				// capture events
				for(const auto& s : streams_) {
					// read maximum number of events from stream
					tmp.resize(num);
					s.read(tmp);
					// set correct ID
					for(auto& e : tmp) {
						e.id = id;
					}
					// add events
					v.insert(v.end(), tmp.begin(), tmp.end());
					id ++;
				}
				// sort events into correct order
				std::sort(v.begin(), v.end(),
					[](const edvs_event_t& a, const edvs_event_t& b) {
						return a.t < b.t;
					});
			}
		}

		void read(std::vector<edvs_event_t>& v, std::vector<edvs_special_t>& v_special) const {
			if(streams_.size() == 0) {
				return;
			}
			else if(streams_.size() == 1) {
				streams_.front().read(v, v_special);
			}
			else {
				// NOT IMPLEMENTED
			}
		}

		std::vector<edvs_event_t> read() const {
			if(streams_.size() == 0) {
				return {};
			}
			else if(streams_.size() == 1) {
				return streams_.front().read();
			}
			else {
				std::vector<edvs_event_t> v(streams_.size() * 1024);
				read(v);
				return v;
			}
		}

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
