#ifndef INCLUDE_EDVS_EVENTIO_HPP
#define INCLUDE_EDVS_EVENTIO_HPP

#include "Event.hpp"
#include <string>
#include <vector>

namespace Edvs
{

	void SaveEvents(const std::string& fn, const std::vector<Event>& v);

	std::vector<Event> LoadEvents(const std::string& fn);

}

#endif
