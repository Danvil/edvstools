# Tools for the Embedded Dynamic Vision Sensor

This library is a collection of small tools for the embedded dynamic vision sensor.

* Edvs: eDVS connection handling (network and serial), saving/loading event files, sensor model
* TestConnection: Connects to an eDVS sensor and computes event rate
* ShowRetina: Connects to an eDVS sensor and displays events
* RecordEvents: Connects to an eDVS sensor and records events to a file
* CheckEvents: Checks timestamps in an event file
* ConvertEvents: Converts saved event files between different data formats
* EventViewer: Displays events stored in an event file
* EventVideoGenerator: Generates a video from a saved event file


# Installation

## Requirements

* [Boost](http://www.boost.org/) 1.46.1 or higher: `sudo apt-get install libboost-all-dev`
* [Eigen](http://eigen.tuxfamily.org) 3.x: `sudo apt-get install libeigen3-dev`
* [Qt](http://qt.nokia.com/) 4.x: `sudo apt-get install libqt4-dev`
* Build essentials: `sudo apt-get install g++ build-essential cmake cmake-qt-gui`

All apt-get dependencies in one line: `sudo apt-get install libboost-all-dev libeigen3-dev libqt4-dev g++ build-essential cmake cmake-qt-gui`

## Installation Instructions

1. `git clone git://github.com/Danvil/edvstools.git`
2. `cd edvstools; mkdir build; cd build`
3. `cmake -DCMAKE_BUILD_TYPE=Release ..`
4. `make`


# Usage

## Displaying events

To connect to the eDVS sensor over network and display events

	ShowRetina/ShowRetina --uri 192.168.201.62:56000

To connect to the eDVS sensor over serial port and display events

	ShowRetina/ShowRetina --uri /dev/ttyUSB0?baudrate=4000000

To replay an previously saved event file

	ShowRetina/ShowRetina --uri /path/to/events

## Capturing events (C++)

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

## Capturing events (C)

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
