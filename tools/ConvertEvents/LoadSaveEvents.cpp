/*
 * IO.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: david
 */

#include "LoadSaveEvents.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Edvs
{

struct TimeUnroller
{
	TimeUnroller()
	: current_time_(0), last_time_(0)
	{}

	uint64_t operator()(uint64_t t) {
		if(current_time_ == 0 && last_time_ == 0) {
			last_time_ = t;
		}
		uint64_t dt;
		if(last_time_ <= t) {
			dt = t - last_time_;
		}
		else {
			dt = 2*t; // HACK we do not know when it wraps!
		}
		last_time_ = t;
		current_time_ += dt;
		return current_time_;
	}

private:
	uint64_t current_time_;
	uint64_t last_time_;
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

bool jc_has_id(const std::string& line)
{
	return line.length() >= 21;
}

std::vector<Edvs::Event> LoadEventsJC(const std::string& filename, bool unwrap_timestamps)
{
	std::vector<Event> events;
	events.reserve(100000);
	// 012345678901234567890
	// 0 0  54  60  38928595
	std::ifstream ifs(filename);
	if(!ifs.is_open()) {
		std::cerr << "Could not open file!" << std::endl;
		return {};
	}
	std::string line;
	Event e;
	TimeUnroller unroller;
	// bool first_line = true;
	// uint64_t start_time;
	while(getline(ifs, line)) {
		bool has_id = jc_has_id(line);
		if(has_id) {
			e.id = static_cast<uint8_t>(line[0] - '0');
			e.parity = (line[2] == '0' ? 0 : 1);
			e.x = static_cast<uint16_t>(parse_coord_fast(line[4], line[5], line[6]));
			e.y = static_cast<uint16_t>(parse_coord_fast(line[8], line[9], line[10]));
			e.t = parse_timestamp_fast(line.rbegin(), line.rbegin() + 8);
		}
		else {
			e.id = 0;
			e.x = static_cast<uint16_t>(parse_coord_fast(line[0], line[1], line[2]));
			e.y = static_cast<uint16_t>(parse_coord_fast(line[4], line[5], line[6]));
			e.parity = (line[8] == '0' ? 0 : 1);
			e.t = parse_timestamp_fast(line.rbegin(), line.rbegin() + 8);
		}
		if(unwrap_timestamps) {
			e.t = unroller(e.t);
		}
		// if(flags & EventFileFlag::BeginTimeZero) {
		// 	if(first_line)
		// 	{
		// 		first_line = false;
		// 		start_time = e.t;
		// 	}
		// 	e.t -= start_time;
		// }
		events.push_back(e);
	}
	return events;
}

std::vector<Edvs::Event> LoadEventsOld(const std::string& filename, bool unwrap_timestamps)
{
	std::vector<Event> events;
	events.reserve(100000);
	std::ifstream ifs(filename, std::ios::binary);
	char buff[21];
	TimeUnroller unroller;
	while(!ifs.eof()) {
		ifs.read((char*)buff, 21);
		Event e;
		e.id = *reinterpret_cast<uint32_t*>(buff);
		e.parity = (*reinterpret_cast<uint8_t*>(buff + 4) == 0 ? false : true);
		e.x = *reinterpret_cast<float*>(buff + 5);
		e.y = *reinterpret_cast<float*>(buff + 9);
		e.t = *reinterpret_cast<uint64_t*>(buff + 13);
		if(unwrap_timestamps) {
			e.t = unroller(e.t);
		}
		events.push_back(e);
	}
	return events;
}

std::vector<Event> LoadEventsTable(const std::string& filename, char separator)
{
	std::string separators;
	separators.push_back(separator);
	std::vector<Event> events;
	events.reserve(100000);
	std::ifstream is(filename);
	std::string line;
	std::vector<std::string> tokens;
	Event e;
	while(getline(is, line)) {
		// eat \r at the end of line
		if(line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1, 1);
		}
		// parse line
		boost::split(tokens, line, boost::is_any_of(separators));
		e.t = boost::lexical_cast<uint64_t>(tokens[0]);
		e.x = boost::lexical_cast<uint16_t>(tokens[1]);
		e.y = boost::lexical_cast<uint16_t>(tokens[2]);
		e.parity = boost::lexical_cast<unsigned>(tokens[3]);
		e.id = boost::lexical_cast<unsigned>(tokens[4]);
		// std::istringstream iss(line);
		// iss >> e.t >> e.x >> e.y >> e.parity >> e.id;
		events.push_back(e);
	}
	return events;
}

std::vector<Edvs::Event> LoadEventsEBSLAM3(const std::string& filename)
{
	std::vector<Event> events;
	events.reserve(100000);
	std::ifstream is(filename);
	std::string line;
	std::vector<std::string> tokens;
	Event e;
	while(getline(is, line)) {
		// eat \r at the end of line
		if(line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1, 1);
		}
		// parse line
		boost::split(tokens, line, boost::is_any_of("\t"));
		e.t = boost::lexical_cast<uint64_t>(tokens[5]);
		e.x = boost::lexical_cast<uint16_t>(tokens[3]);
		e.y = boost::lexical_cast<uint16_t>(tokens[4]);
		e.parity = boost::lexical_cast<unsigned>(tokens[6]);
		e.id = 0;
		// std::istringstream iss(line);
		// iss >> e.t >> e.x >> e.y >> e.parity >> e.id;
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

}
