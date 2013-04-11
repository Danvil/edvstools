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
	struct EventCaptureImpl;

	/** Handle for edvs device and event capture */
	struct DeviceHandle
	{
		boost::shared_ptr<Device> device;
		boost::shared_ptr<EventCaptureImpl> capture_impl;
	};

	enum Baudrate
	{
		B921k, B1000k, B2000k, B4000k
	};

	/** Edvs connection over serial port
	 * The device is opened on object construction and closed on object destruction.
	 * Please assure that you now how shared_ptr works!
	 * @param baudrate Edvs baudrate
	 * @param port Link to serial port, e.g. /dev/ttyUSB0
	 * @return link to device
	 */
	DeviceHandle OpenSerialDevice(Baudrate baudrate, std::string port="/dev/ttyUSB0");

	/** Edvs connection over network socket
	 * Please assure that you now how shared_ptr works!
	 * The device is opened on object construction and closed on object destruction.
	 * @param address Edvs network addresse of the form ip:port
	 * @return link to device
	 */
	DeviceHandle OpenNetworkDevice(const std::string& address);

	/** Type of event callback function */
	typedef boost::function<void(const std::vector<RawEvent>&)> EventCallbackType;

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
	void StartEventCapture(DeviceHandle& handle, EventCallbackType callback, size_t buffer_size = 8192);

}

#endif
