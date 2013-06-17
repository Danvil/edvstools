#ifndef INCLUDE_EDVS_EVENTSTREAM_HPP
#define INCLUDE_EDVS_EVENTSTREAM_HPP

#include "edvs.h"
#include "Event.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <string>
#include <vector>

namespace Edvs
{

	class EventStream
	{
	private:
		struct Impl
		{
			Impl(const std::string& uri) {
				h = edvs_open(uri.c_str());
			}
			~Impl() {
				if(h) edvs_close(h);
			}
			edvs_stream_handle h;
		};

	public:
		EventStream() {}
		
		EventStream(const std::string& uri) {
			open(uri);
		}

		void open(const std::string& uri) {
			impl_ = boost::make_shared<Impl>(uri);
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
		boost::shared_ptr<Impl> impl_;
	};

}

#endif
