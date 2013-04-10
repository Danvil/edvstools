/*
 * Edvs.h
 *
 *  Created on: Mar 23, 2011
 *  Changed on: Mar 25, 2011
 *      Author: David Weikersdorfer
 *
 * C++ classes and functions to access the eDVS128 sensor
 *
 * The code has been tested under Ubuntu 10.04 with gcc 4.4.3
 *
 * Requirements:
 *   -- boost --
 *   All headers are required, but you only need to compile
 *   the library boost_thread library.
 *   Please add the boost include directory to your include
 *   path and the boost lib directory to your library path.
 *   Example: -I$BOOST_INCLUDE -L$BOOST_LOB -lboost_thread
 *
 * Code example:
 *

// header with Edvs functions
#include "Edvs.h"
// to write/read to/from the console
#include <iostream>

// C style event callback function
void OnEvent(const std::vector<Edvs::Event>& events)
{
	std::cout << "Got " << events.size() << " events: ";
	// iterating over the events and writing them to the console
	for(std::vector<Edvs::Event>::const_iterator it=events.begin(); it!=events.end(); it++) {
		// int x = it->x; int y = it->y; bool parity = it->parity
		std::cout << *it << ", ";
	}
	std::cout << std::endl;
}

// the main program
int main(int argc, char* argv[])
{
	// open device and specify connection speed
	Edvs::Device device(Edvs::B4000k);
	// start event capture
	Edvs::EventCapture capture(device, OnEvent);
	// read in string from the console and post them as command strings
	// try e.g. 0/1/2 to change the LED state (off/on/blicking)
	// press q to quit the program
	std::string str;
	while(str != "q") {
		std::cin >> str;
		device.WriteCommand(str + "\n");
	}
	return 0;
}

 *
 * Example how to change the device speed:
 * This code changes the device baud rate.
 * It's inside brackets so that the device connection
 * is closed immediately. We cannot use this connection
 * to access data because its connection speed is not
 * changed automatically.
 *
	{ // set rate to 4M, keep the brackets!
		Edvs::Device m;
		m.WriteCommand("!S=4000000\n");
	}

 */

#ifndef INCLUDE_EDVS_EVENTCAPTURE_HPP_
#define INCLUDE_EDVS_EVENTCAPTURE_HPP_

#include "Event.hpp"
#include "Device.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>

namespace Edvs
{
	enum Baudrate {
		B921k, B1000k, B2000k, B4000k
	};

	/** Edvs connection over serial port
	 * The device is opened on object construction and closed on object destruction.
	 * Please assure that you now how shared_ptr works!
	 * @param baudrate Edvs baudrate
	 * @param port Link to serial port, e.g. /dev/ttyUSB0
	 * @return link to device
	 */
	boost::shared_ptr<Device> CreateSerialDevice(
		Baudrate baudrate,
		std::string port="/dev/ttyUSB0"
	);

	/** Edvs connection over network socket
	 * Please assure that you now how shared_ptr works!
	 * The device is opened on object construction and closed on object destruction.
	 * @param address Edvs network addresse of the form ip:port
	 * @return link to device
	 */
	boost::shared_ptr<Device> CreateNetworkDevice(
		const std::string& address
	);

	/** Type of event callback function */
	typedef boost::function<void(const std::vector<RawEvent>&)> EventCallbackType;

	struct EventCaptureImpl;

	struct EventCapture
	{
		EventCapture(const boost::shared_ptr<Device>& device, EventCallbackType f, size_t buffer_size);
	private:
		boost::shared_ptr<EventCaptureImpl> capture_;
	};

	/** Runs event capture
	 * Capturing starts on object construction and ends on object destruction.
	 * Please assure that you now how shared_ptr works!
	 * Capturing runs in a separate thread and calls the callback
	 * function when new events are received.
	 * Please assure that the callback function is thread safe!
	 * Do not perform computational intensive operations in the callback function
	 * otherwise you will not be able to receive all events.
	 * @param device Edvs device handle
	 * @param callback Edvs callback function
	 * @param buffer_size Maximum bytes to read from the device at once
	 */
	inline boost::shared_ptr<EventCapture> RunEventCapture(
		const boost::shared_ptr<Device>& device,
		EventCallbackType callback,
		size_t buffer_size = 8192
	) {
		return boost::make_shared<EventCapture>(device, callback, buffer_size);
	}

}

#endif
