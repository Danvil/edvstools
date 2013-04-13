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
				return;
			}
			ssize_t m = edvs_read(impl_->h, v.data(), v.size());
			v.resize(m);
		}

		std::vector<edvs_event_t> read() const {
			std::vector<edvs_event_t> v;
			if(!is_open()) {
				return v;
			}
			size_t num = 1024; // FIXME how many can we read?
			v.resize(num);
			read(v);
			return v;
		}

	private:
		boost::shared_ptr<Impl> impl_;
	};

}

#endif
