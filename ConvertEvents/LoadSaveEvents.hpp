/*
 * IO.hpp
 *
 *  Created on: Jun 28, 2012
 *      Author: david
 */

#ifndef EDVS_TOOLS_IO_HPP_
#define EDVS_TOOLS_IO_HPP_

#include "Event.hpp"
#include <string>
#include <vector>

namespace Edvs
{
	/** Loads events in table file format
	 * Each line is one event and lines are separated with std::endl.
	 * Each value in each line is separated by 'separator'
	 * Values per line: t, x, y, parity, id
	 */
	std::vector<Edvs::Event> LoadEventsTable(const std::string& filename, char separator);

	/** Loads events from JC file format
	 * Each line is one event.
	 * Line format: XXX YYY P TTTTTTTTTT
	 */
	std::vector<Edvs::Event> LoadEventsJC(const std::string& filename, bool unwrap_timestamps=false);

	/** Loads events from an old binary file format
	 * 		for each event 21 bytes of binary data
	 * 		0-3: id as uint32_t
	 * 		4: parity as uint8_t
	 * 		5-8: x as float
	 * 		9-12: y as float
	 * 		13-20: time as uint64_t
	 */
	std::vector<Edvs::Event> LoadEventsOld(const std::string& filename, bool unwrap_timestamps=false);

	/** Saves a list of events in a file
	 */
	void SaveEventsTable(const std::string& filename, const std::vector<Edvs::Event>& events, char separator);

}

#endif
