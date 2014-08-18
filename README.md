# edvstools

*Copyright 2012-2014 by David Weikersdorfer*

This library is a collection of tools for the embedded Dynamic Vision Sensor [eDVS](http://www.nst.ei.tum.de/projekte/edvs/). The Dynamic Vision Sensor also known as [siliconretina](http://siliconretina.ini.uzh.ch/wiki/index.php) is a asynchronous event-based vision sensor developed at the University of Zurich.

**Tools / Libraries**

* libEdvs -- access events from eDVS, serial port or network port connection handling, save/load event files
* ShowEvents -- live visualization of events, save to a file and view saved event files
* ConvertEvents -- converts saved event files between different data formats
* EventVideoGenerator -- generates an mpeg video from a saved event file


## Installation

edvstools was tested under Ubuntu 14.04 with GCC 4.8.x. It should also run with older versions.

### Requirements

* Build essential: `sudo apt-get install build-essential g++ cmake cmake-qt-gui`
* [Boost](http://www.boost.org/) 1.48 or higher: `sudo apt-get install libboost-all-dev`
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

Open two edvs streams using master/slave mode

	bin/ShowEvents --uri /dev/ttyUSB0?baudrate=4000000\&msmode=1 /dev/ttyUSB1?baudrate=4000000\&msmode=2

To replay an previously saved event file

	bin/ShowEvents --uri /path/to/eventfile

To connect to the eDVS sensor over network and display events

	bin/ShowEvents --uri 192.168.201.62:56000

### Convert files from binary into TSV format

ShowEvents saves events in a binary file format to create smaller files which can be saved and loaded quicker. If an ASCII text file is required for an external program, the tool ConvertEvents can be used to convert binary event files into TSV text files.

    bin/ConvertEvents --in /path/to/eventfile --out /path/to/eventfile.tsv

Try `bin/ConvertEvents --h` for more options.

### Creating an event video

edvstools comes with the tool EventVideoGenerator which can create videos for a recorded event file. EventVideoGenerator only works with binary event files.

    mkdir /tmp/eventvideo
    bin/EventVideoGenerator --fn /path/to/eventfile --dir /tmp/eventvideo

This writes all video frames as PNG images to the specified directory. To create the video proceed as indicated in the program output and execute for example:

    cd /tmp/eventvideo/
    mogrify -format jpg *.png
    avconv -f image2 -r 25 -i %05d.jpg -c:v libx264 -r 25 video.mp4

This requires `libav-tools` and `imagemagick`.

Try `bin/EventVideoGenerator --h` for more options.

## Troubleshooting

#### I can not open event files

Event files are binary files. Use ConvertEvents to generate a TSV file. The binary file format is simply a binary dump of an array of edvs_event_t.

#### I do not get correct timestamps

Make sure you use the correct URI option dtsm (see URI format below).
Make sure you escape the `&` sign as `\&` when entered in a linux terminal!

#### I get "Error in high bit! Skipping a byte" and my events look strange

Make sure that your edvs firmware uses the expected event file format. The current version of libEdvs expects the format `1yyyyyyy pxxxxxxx` (the test bit is 1!). See Edvs/edvs.c at line 549.

Check if your edvs firmware supports differential timestamps. The introduction of differential timestamps changed the semantic of !E1, !E2 and !E3. If your firmware supports differential timestamps the following patch should help:

    --- Edvs/edvs.c 2014-08-05 17:55:39.302022615 +0200
    +++ /tmp/edvs.c 2014-08-17 17:42:51.417998211 +0200
    @@ -271,15 +271,15 @@
        sleep_ms(200);
        // timestamp mode
        if(s->device_timestamp_mode == 1) {
    -       if(edvs_device_write_str(dh, "!E1\n") != 0)
    +       if(edvs_device_write_str(dh, "!E2\n") != 0)
                return 0;
        }
        else if(s->device_timestamp_mode == 2) {
    -       if(edvs_device_write_str(dh, "!E2\n") != 0)
    +       if(edvs_device_write_str(dh, "!E3\n") != 0)
                return 0;
        }
        else if(s->device_timestamp_mode == 3) {
    -       if(edvs_device_write_str(dh, "!E3\n") != 0)
    +       if(edvs_device_write_str(dh, "!E4\n") != 0)
                return 0;
        }
        else {

Save to a file '/tmp/diffs.patch' and apply with

    cd ~/git/edvstools
    patch -p0 < /tmp/diffs.patch

## URI format

Most edvs tools use an [URI](http://en.wikipedia.org/wiki/URI_scheme) to indicate how the edvs device/file should be opened. An edvs URI has the format `LINK?OPT1=VAL1&OPT2=VAL2&...&OPTn=VALn`. Key/value pairs after the `?` are optional.
There are tree edvs URI types -- serial port, file, network -- explained in the following.

**Important note:** The `&` character needs to be escaped as `\&` when entered at a linux terminal. So you have to type

    bin/tool --uri /dev/ttyUSB0?baudrate=4000000\&dtsm=2
                                               ^^^
instead of

    bin/tool --uri /dev/ttyUSB0?baudrate=4000000&dtsm=2

### Serial port

Format: `DEVICE?baudrate=BAUD&dtsm=DTSM&htsm=HTSM&msmode=MSM`
* DEVICE -- path to device, i.e. /dev/ttyUSB0
* BAUD -- serial port baudrate, i.e. 4000000 (*default is 4000000*)
* DTSM -- device timestamp mode
 * 0: no timestamps (sends `!E0`)
 * 1: 16-bit timestamps (sends `!E1`)
 * 2: 24-bit timestamps (sends `!E2`) (*default*)
 * 3: 32-bit timestamps (sends `!E3`)
* HTSM -- host timestamp mode
 * 0: use raw timestamps from device
 * 1: unwrap timestamps to guarantee monotonic increasing timestamps (*default*)
 * 2: (experimental) use a combination of system time and device timestamps
* MSM -- master slave mode
 * 0: disable master/slave mode (*default*)
 * 1: operate as master (sends `!ETM0` and later `!ETM+`)
 * 2: operate as slave (sends `!ETS`)

Example: `/dev/ttyUSB0?baudrate=4000000\&dtsm=2\&htsm=1\&msmode=0`


### File

Format: `PATH?dt=DT&ts=TS`
* PATH -- path to file
* DT -- time in microseconds to add for each call to get events
  * 0 -- system time is used to add the elapsed time since the last call; useful if realtime behaviour is desired (*default*)
  * >0 -- fixed amount of DT is added; useful when events are processed much slower than they are captured
* TS -- only if DT=0: scales the elapsed delta system time by the specified value (*default is 1.0*)

Example: `/path/to/eventfile?dt=0\&ts=0.5`

### Network

Format: `IP:PORT?dtsm=DTSM&htsm=HTSM&msmode=MSM`
* IP -- network ip of edvs
* PORT --- network port number of edvs
* DTSM, HTSM and MSM identical to the serial port options

Example: `192.168.201.62:56000?baudrate=4000000\&dtsm=2\&htsm=1\&msmode=0`

## Code examples

### Capturing events (C++)

The following sample demonstrates how to open and event stream and read events using the C++ interface.

	#include <Edvs/EventStream.hpp>
	#include <iostream>

	int main(int argc, char* argv[])
	{
		// open stream (use first command line parameter as URI)
		std::shared_ptr<Edvs::IEventStream> stream = Edvs::OpenEventStream(argv[1]);
		// capture events (run until end of file or Ctrl+C)
		while(!stream->eos()) {
			// read events from stream
			auto events = stream->read();
			// display message
			if(!events.empty()) {
				std::cout << "Time " << events.back().t << ": " << events.size() << " events" << std::endl;
			}
		}
		return 1;
	}



### Capturing events (C)

The following sample demonstrates how to read edvs events in C.

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

edvstools is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

edvstools is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with asp. If not, see [www.gnu.org/licenses](http://www.gnu.org/licenses/).
