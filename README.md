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

	ShowRetina/ShowRetina --device net --link 192.168.201.62:56001

To connect to the eDVS sensor over serial port and display events

	ShowRetina/ShowRetina --device serial --link /dev/ttyUSB0

## Capturing events (C++)

The following sample demonstrates the usage of the C++ edvs event stream.

	#include <Edvs/EventCapture.hpp>
	#include <boost/bind.hpp>
	#include <iostream>

	// handle events
	void OnEvent(const std::vector<Edvs::Event>& events)
	{
		std::cout << "Got " << events.size() << " events: ";
		for(std::vector<Edvs::RawEvent>::const_iterator it=events.begin(); it!=events.end(); it++) {
			std::cout << *it << ", ";
		}
		std::cout << std::endl;
	}

	// main loop
	int main(int argc, char* argv[])
	{
		// run capture
		Edvs::EventStream stream("192.168.201.62:56000");
		Edvs::EventCapture capture(stream, &OnEvent);
		// press q to quit
		std::string str;
		while(str != "q") {
			// read command
			std::cin >> str;
		}
		return 1;
	}

## Capturing events (C)

The following sample demonstrates the usage of the C++ edvs event stream.

// #include <signal.h>
// #include <stdio.h>

	int main(int argc, char** argv)
	{
		if(argc != 2) {
			printf("Wrong usage\n");
			return EXIT_FAILURE;
		}
		edvs_stream_handle s = edvs_open(argv[1]);
		size_t n = 128;
		edvs_event_t* events = (edvs_event_t*)malloc(n*sizeof(edvs_event_t));
		size_t num = 0;
		while(num < 10000) {
			ssize_t m = edvs_read(s, events, n);
			printf("%zd/%d events\n", num, 10000);
			num += m;
		}
		edvs_close(s);
		free(events);
		return EXIT_SUCCESS;
	}
