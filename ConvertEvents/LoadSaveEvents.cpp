/*
 * IO.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: david
 */

#include "LoadSaveEvents.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Edvs
{

struct EventLineEntry
{
	uint64_t t;
	uint16_t x, y;
	bool flank;
	int c1, c2, c3;
};

EventLineEntry ParseEventLine(const std::string& line, unsigned int version)
{
	if(version == 0) {
		EventLineEntry e;
		// tab separated values
		//3976	98.4259809	148.546181	1	-4	20	0
		std::vector<std::string> tokens;
		boost::split(tokens, line, boost::is_any_of("\t"));
		e.t = boost::lexical_cast<int>(tokens[0]);
		e.x = boost::lexical_cast<float>(tokens[1]);
		e.y = boost::lexical_cast<float>(tokens[2]);
		e.flank = boost::lexical_cast<bool>(tokens[3]);
		e.c1 = boost::lexical_cast<int>(tokens[4]);
		e.c2 = boost::lexical_cast<int>(tokens[5]);
		e.c3 = boost::lexical_cast<int>(tokens[6]);
		return e;
	}
	else if(version == 1) {
		EventLineEntry e;
		std::istringstream iss(line);
		iss >> e.x;
		iss >> e.y;
		float flank;
		iss >> flank;
		e.flank = flank > 0;
		float time;
		iss >> time;
		e.t = time;
//		std::cout << e.t << " (" << e.x << "," << e.y << ") " << e.flank << std::endl;
		e.c1 = 0;
		e.c2 = 0;
		e.c3 = 0;
		return e;
	}
}

std::vector<Event> LoadEventsMisc(const std::string& filename, unsigned int version)
{
	std::vector<Event> events;
	std::ifstream is(filename.c_str());
	std::string line;
	events.clear();
	while(getline(is, line)) {
		if(line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1, 1);
		}
		// parse line
		EventLineEntry e = ParseEventLine(line, version);
		Event event{e.t, e.x, e.y, e.flank, 0};
		events.push_back(event);
	}
	return events;
}

bool isBinary(const std::string& filename, EventFileFlags format)
{
	if(format & EventFileFlag::Text) {
		return false;
	}
	else if(format & EventFileFlags::Binary) {
		return true;
	}
	else {
		boost::filesystem::path p(filename);
		return (p.extension().string() != ".txt");
	}
}

struct TimeUnroller
{
	TimeUnroller()
	: last_time_(0), time_add_(0)
	{}

	uint64_t operator()(uint64_t t) {
		if(t < last_time_ && std::abs(last_time_-t) > 60108864) {
			time_add_ += 67108864;
		}
		last_time_ = t;
		return t + time_add_;
	}

private:
	uint64_t last_time_;
	uint64_t time_add_;
};

inline uint32_t parse_coord_fast(unsigned char c, unsigned char b, unsigned char a) {
	return                static_cast<uint32_t>(a - '0')
	+ (b == ' ' ? 0 :  10*static_cast<uint32_t>(b - '0'))
	+ (c == ' ' ? 0 : 100*static_cast<uint32_t>(c - '0'));
}

template<typename StrIt>
inline uint64_t parse_timestamp_fast(StrIt it, const StrIt& end) {
	uint64_t result = 0;
	uint64_t mult = 1;
	while(it != end && *it < '0' && '9' < *it) ++it;
	while(it != end && *it != ' ') {
		result += mult*static_cast<uint64_t>(*it - '0');
		mult *= 10;
		++it;
	}
	return result;
}

std::vector<Event> LoadEventSVar(const std::string& filename, EventFileFlags flags)
{
	bool fix_wrapped_timestamps = flags & EventFileFlag::UnwrapTimestamps;
	std::vector<Event> events;
	events.reserve(100000);
	TimeUnroller unroller;
	if(isBinary(filename, flags)) {
		std::ifstream ifs(filename, std::ios::binary);
		char buff[21];
		while(!ifs.eof()) {
			ifs.read((char*)buff, 21);
			Event e;
			e.id = *reinterpret_cast<uint32_t*>(buff);
			e.parity = (*reinterpret_cast<uint8_t*>(buff + 4) == 0 ? false : true);
			e.x = *reinterpret_cast<float*>(buff + 5);
			e.y = *reinterpret_cast<float*>(buff + 9);
			e.t = *reinterpret_cast<uint64_t*>(buff + 13);
			if(fix_wrapped_timestamps) {
				e.t = unroller(e.t);
			}
			events.push_back(e);
		}
	}
	else {
		if(flags & EventFileFlag::RawWithId) {
			// 012345678901234567890
			// 0 0  54  60  38928595
			std::ifstream is(filename);
			std::string line;
			Event e;
			
			bool first_line = true;
			uint64_t start_time;

			while(getline(is, line)) {
				e.id = static_cast<uint32_t>(line[0] - '0');
				e.parity = (line[2] == '0' ? false : true);
				e.x = static_cast<float>(parse_coord_fast(line[4], line[5], line[6]));
				e.y = static_cast<float>(parse_coord_fast(line[8], line[9], line[10]));
				e.t = parse_timestamp_fast(line.rbegin(), line.rbegin() + 8);
				if(fix_wrapped_timestamps) {
					e.t = unroller(e.t);
				}
				if(flags & EventFileFlag::BeginTimeZero) {
					if(first_line)
					{
						first_line = false;
						start_time = e.t;
					}
					e.t -= start_time;
				}
				events.push_back(e);
			}
		}
		else {

			std::ifstream is(filename);
			std::string line;
			while(getline(is, line)) {
				if(line[line.size() - 1] == '\r') {
					line.erase(line.size() - 1, 1);
				}
				// parse line
				std::istringstream iss(line);
				Event e;
				unsigned int ep;
				iss >> e.id >> ep >> e.x >> e.y >> e.t;
				e.parity = (ep == 0 ? false : true);
				if(fix_wrapped_timestamps) {
					e.t = unroller(e.t);
				}
				events.push_back(e);
			}

			// std::ifstream is(filename);
			// std::string line;
			// std::vector<std::string> tokens;
			// while(getline(is, line)) {
			// 	// if(line[line.size() - 1] == '\r') {
			// 	// 	line.erase(line.size() - 1, 1);
			// 	// }
			// 	boost::split(tokens, line, boost::is_any_of(" \t"), boost::token_compress_on);
			// 	for(const std::string& t : tokens) {
			// 		std::cout << "'" << t << "'" << std::endl;
			// 	}
			// 	Event e;
			// 	e.id = boost::lexical_cast<uint32_t>(tokens[0]);
			// 	unsigned int ep = boost::lexical_cast<unsigned char>(tokens[1]);
			// 	e.x = boost::lexical_cast<float>(tokens[2]);
			// 	e.y = boost::lexical_cast<float>(tokens[3]);
			// 	e.t = boost::lexical_cast<uint64_t>(tokens[4]);
			// 	e.parity = (ep == 0 ? false : true);
			// 	if(fix_wrapped_timestamps) {
			// 		e.t = unroller(e.t);
			// 	}
			// 	events.push_back(e);
			// }

		}
	}
	return events;
}

std::vector<Event> LoadEventsMatlab(const std::string& filename)
{
	std::vector<Event> events;
	events.reserve(100000);
	std::ifstream is(filename);
	std::string line;
	Event e;
	while(getline(is, line)) {
		if(line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1, 1);
		}
		// parse line
		std::istringstream iss(line);
		double et, ex, ey, ep;
		iss >> ex >> ey >> ep >> et;
		e.id = 0;
		e.parity = (std::abs(ep - 1.0f) < 0.00001f ? true : false);
		e.x = ex;
		e.y = ey;
		e.t = static_cast<uint64_t>(et);
		events.push_back(e);
	}
	return events;
}

void SaveEventsTable(const std::string& filename, const std::vector<Edvs::Event>& events, char sep)
{
	std::ofstream ofs(filename);
	for(const Event& e : events) {
		ofs << e.t << sep
			<< e.x << sep
			<< e.y << sep
			<< (unsigned int)e.parity << sep
			<< (unsigned int)e.id << std::endl;
	}
}

// void SaveEvents(const std::vector<Event>& events, const std::string& filename, EventFileFlags flags)
// {
// 	if(isBinary(filename, flags)) {
// 		std::ofstream ofs(filename, std::ios::binary);
// 		for(const Event& e : events) {
// 			ofs.write(reinterpret_cast<const char*>(&e.id), 4);
// 			uint8_t parity = e.parity ? 1 : 0;
// 			ofs.write(reinterpret_cast<const char*>(&parity), 1);
// 			ofs.write(reinterpret_cast<const char*>(&e.x), 4);
// 			ofs.write(reinterpret_cast<const char*>(&e.y), 4);
// 			ofs.write(reinterpret_cast<const char*>(&e.t), 8);
// 		}
// 	}
// }

}
