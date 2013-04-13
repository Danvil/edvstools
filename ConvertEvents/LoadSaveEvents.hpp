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

	std::vector<Edvs::Event> LoadEventsMisc(const std::string& filename, unsigned int version);

	namespace EventFileFlag
	{
		enum Type {
			None = 0,
			Text = 1<<0,
			Binary = 1<<1,
			RawWithId = 1<<2,
			UnwrapTimestamps = 1<<3,
			BeginTimeZero = 1<<4
		};
	}
	typedef EventFileFlag::Type EventFileFlags;

	/** Loads a list of events from a file
	 * An event consists of
	 * 	a timestamp (positive integer),
	 * 	the x/y event pixel position on the retina (integer in range 0-128) and
	 * 	a parity value which is 0 or 1.
	 * There are three format specifiers: Text, Binary or Auto
	 * 		Text/Binary saves events in text/binary format
	 * 		Auto determines format by filename. An extension of txt indicates
	 * 		a text file, all other extensions indicate binary files.
	 * There are two file formats text and binary:
	 * 	a) Text
	 * 		each line consists of one event of the following format
	 * 			id:uint32_t
	 * 			parity:uint8_t
	 * 			x:float
	 * 			y:float
	* 			time:uint64_t
	 * 		the inidividual entries are separated by tabs
	 * 	b) Binary
	 * 		for each event 18 bytes of binary data
	 * 		0-4: id as uint32_t
	 * 		5: parity as uint8_t
	 * 		6-9: x as float
	 * 		10-13: y as float
	 * 		14-21: time as uint64_t
	 */
	std::vector<Edvs::Event> LoadEventsVar(const std::string& filename, EventFileFlags flags=EventFileFlag::UnwrapTimestamps);

	/* Loads a list of events from a file in an old file format
	 * File format:
	 * 		each line consists of one event of the following format
	 *		x: float	y: float	parity: float	time: float
	 * 		the inidividual entries are separated by tabs
	 */
	std::vector<Edvs::Event> LoadEventsMatlab(const std::string& filename);

	/** Saves a list of events in a file
	 */
	void SaveEventsTable(const std::string& filename, const std::vector<Edvs::Event>& events, char separator);

}

#endif
