# edvstools

This library is a collection of tools for the embedded Dynamic Vision Sensor [eDVS](http://www.nst.ei.tum.de/projekte/edvs/). The Dynamic Vision Sensor also known as [siliconretina](http://siliconretina.ini.uzh.ch/wiki/index.php) is a asynchronous event-based vision sensor developed at the University of Zurich.

**Tools / Libraries**

* libEdvs -- access events from eDVS, serial port or network port connection handling, save/load event files
* ShowEvents -- live visualization of events, save to a file and view saved event files
* ConvertEvents -- converts saved event files between different data formats
* EventVideoGenerator -- generates an mpeg video from a saved event file


## Installation

edvstools is was tested under Ubuntu 14.04 with GCC 4.8.x. It should also run with older version.

### Requirements

* Build essential: `sudo apt-get install build-essential g++ cmake cmake-qt-gui`
* [Boost](http://www.boost.org/) 1.46.1 or higher: `sudo apt-get install libboost-all-dev`
* [Eigen](http://eigen.tuxfamily.org) 3.x: `sudo apt-get install libeigen3-dev`
* [Qt](http://qt.nokia.com/) 4.x: `sudo apt-get install libqt4-dev`

All apt-get dependencies in one line: `sudo apt-get install build-essential g++ cmake cmake-qt-gui libboost-all-dev libeigen3-dev libqt4-dev `

### Installation Instructions

1. `git clone git://github.com/Danvil/edvstools.git`
2. `cd edvstools; mkdir build; cd build`
3. `cmake -DCMAKE_BUILD_TYPE=Release ..`
4. `make`


## Usage

### Displaying events

To connect to the eDVS sensor over serial port and display events

	bin/ShowEvents --uri /dev/ttyUSB0?baudrate=4000000

Same but use 24 bit timestamps instead of 16 bit timestamps

	bin/ShowEvents --uri /dev/ttyUSB0?baudrate=4000000\&dtsm=2

To replay an previously saved event file

	bin/ShowEvents --uri /path/to/events

To connect to the eDVS sensor over network and display events

	bin/ShowEvents --uri 192.168.201.62:56000

## URI format

Most edvs tools use a URI to indicate how the edvs device/file should be opened. The URI has the basic format `LINK?OPT1=VAL1&OPT2=VAL2&...&OPTn=VALn`
There are tree URI types -- serial port, file, network -- explained in the following.

**Important note:** The `&` character needs to be escaped as `\&` when entered at a linux terminal. So you have to type

    bin/tool --uri /dev/ttyUSB0?baudrate=4000000\&dtsm=2
                                               ^^^
instead of

    bin/tool --uri /dev/ttyUSB0?baudrate=4000000&dtsm=2

### Serial port

Format: `DEVICE?baudrate=BAUD&dtsm=DTSM&htsm=HTSM&msmode=MSM`
* DEVICE -- path to device, i.e. /dev/ttyUSB0
* BAUD -- serial port baudrate, i.e. 4000000 (default is 4000000)
* DTSM -- device timestamp mode
 * 0: no timestamps (sends `!E0`)
 * 1: 16-bit timestamps (sends `!E1`)  (*default*)
 * 2: 24-bit timestamps (sends `!E2`)
 * 3: 32-bit timestamps (sends `!E3`)
* HTSM -- host timestamp mode
 * 0: use raw timestamps from device (*default*)
 * 1: unwrap timestamps to guarantee monotonic increasing timestamps
 * 2: (experimental) use a combination of system time and device timestamps
* MSM -- master slave mode
 * 0: disable master/slave mode (*default*)
 * 1: operate as master (sends `!ETM0` and later `!ETM+`)
 * 2: operate as slave (sends `!ETS`)

### File

Format: `PATH?dt=DT&ts=TS`
* PATH -- path to file
* DT -- time in microseconds to add for each call to get events
  * 0 -- system time is used to add the elapsed time since the last call (*default*); useful if realtime behaviour is desired
  * >0 -- fixed amount of DT is added; useful when events are processed much slower than they are captured
* TS -- only if DT=0: scales the elapsed delta system time by the specified value (default is 1.0)

### Network

Format: `IP:PORT?dtsm=DTSM&htsm=HTSM&msmode=MSM`
* IP -- network ip of edvs
* PORT --- network port number of edvs
* DTSM, HTSM and MSM identical to the serial port options

## Code examples

### Capturing events (C++)

The following sample demonstrates the usage of the C++ edvs event stream wrapper.

	#include <Edvs/EventCapture.hpp>
	#include <iostream>

	void OnEvent(const std::vector<Edvs::Event>& events)
	{
		if(events.size() > 0) {
			std::cout << "Time " << events.back().t << ": " << events.size() << " events" << std::endl;
		}
	}

	int main(int argc, char* argv[])
	{
		if(argc != 2) {
			std::cout << "Wrong usage" << std::endl;
			return 0;
		}
		// run event capture
		Edvs::EventStream stream(argv[1]);
		Edvs::EventCapture capture(stream, &OnEvent);
		// run until EOF or Ctrl+C
		while(!stream.eos());
		return 1;
	}

### Capturing events (C)

The following sample demonstrates how to read edvs events using plain C.

	#include "edvs.h"
	#include <stdlib.h>
	#include <stdio.h>

	int main(int argc, char** argv)
	{
		if(argc != 2) {
			printf("Wrong usage\n");
			return EXIT_FAILURE;
		}
		// run event capture
		edvs_stream_handle s = edvs_open(argv[1]);
		size_t num_max_events = 1024;
		edvs_event_t* events = (edvs_event_t*)malloc(num_max_events*sizeof(edvs_event_t));
		while(!edvs_eos(s)) {
			ssize_t m = edvs_read(s, events, num_max_events);
			if(m > 0) {
				printf("Time %lu: %zd events\n", events->t, m);
			}
		}
		// cleanup
		edvs_close(s);
		free(events);
		return EXIT_SUCCESS;
	}

## I want to contribute 

Contact me on [GitHub](https://github.com/Danvil/) 

## License

asp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

asp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with asp. If not, see [www.gnu.org/licenses](http://www.gnu.org/licenses/).
