#include "EventIO.hpp"
#include "edvs.h"
#include <stdio.h>

namespace Edvs
{
	void SaveEvents(const std::string& fn, const std::vector<Event>& v)
	{
		FILE* f = fopen(fn.c_str(), "bw");
		edvs_file_write(f, v.data(), v.size());
		fclose(f);
	}

	std::vector<Event> LoadEvents(const std::string& fn)
	{
		FILE* f = fopen(fn.c_str(), "br");
		std::vector<Event> v;
		const size_t num_max = 1024;
		std::vector<edvs_event_t> buffer(num_max);
		while(true) {
			ssize_t m = edvs_file_read(f, buffer.data(), num_max);
			v.insert(v.end(), buffer.begin(), buffer.begin() + num_max);
			if(m != num_max) {
				break;
			}
		}
		fclose(f);
		return v;
	}

}