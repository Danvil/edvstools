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

#ifndef EDVS_H_
#define EDVS_H_

#include "Device.h"
#include "SerialPort.h"
#include "NetSocket.h"
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>

//#define VERBOSE

namespace Edvs
{
	const int cDeviceDisplaySize = 128;

	const bool cEnableFillRead = false;

	inline boost::shared_ptr<SerialPort> create_serial_device(Baudrate br, std::string port="/dev/ttyUSB0")
	{
		return boost::shared_ptr<SerialPort>(new SerialPort(br, port));
	}

	inline boost::shared_ptr<NetSocket> create_network_device(const std::string& address)
	{
		return boost::shared_ptr<NetSocket>(new NetSocket(address));
	}

//	/** An raw eDVS event */
//	struct RawEvent
//	{
//		uint16_t time;
//		uint8_t x, y;
//		bool parity;
//
//		friend inline std::ostream& operator<<(std::ostream& os, const Edvs::RawEvent& e) {
//			os << "[t=" << e.time << ", x=(" << static_cast<unsigned int>(e.x) << ", " << static_cast<unsigned int>(e.y) << "), p=" << e.parity << "]";
//			return os;
//		}
//
//	};

	/** An eDVS event */
	struct RawEvent
	{
		// timestamp
		uint32_t time;

		// event pixel coordinate
		unsigned char x, y;

		// parity
		bool parity;

		friend inline std::ostream& operator<<(std::ostream& os, const Edvs::RawEvent& e) {
			os << "[t=" << e.time << ", p=" << e.parity << ", x=(" << static_cast<int>(e.x) << ", " << static_cast<int>(e.y) << ")]";
			return os;
		}

	};

	struct Event
	{
		// timestamp
		uint64_t time;

		// device id
		uint32_t id;

		// event pixel coordinate
		float x, y;

		// parity
		bool parity;

		friend inline std::ostream& operator<<(std::ostream& os, const Edvs::Event& e) {
			os << "[t=" << e.time << ", id=" << e.id << ", p=" << e.parity << ", x=(" << e.x << ", " << e.y << ")]";
			return os;
		}

	};

	/** Type of event callback function */
	typedef boost::function<void(const std::vector<RawEvent>&)> EventCallbackType;

	namespace Impl
	{
		/** Event capturing */
		struct EventCapture
		{
			EventCapture() {
			}
			EventCapture(const boost::shared_ptr<Device>& device, EventCallbackType f, size_t buffer_size)
			: device_(device), event_callback_(f), buffer_size_(buffer_size), running_(false) {
				StartEventCapture();
			}
			~EventCapture() {
				StopEventCapture();
			}
		private:
			static const unsigned int cTimestampMode = 3;
			void StartEventCapture() {
				if(running_) {
					throw "Already running!";
				}
				// E0: no timestamps, E1: 16 bit timestamps, E2: 24 bit timestamps
				if(cTimestampMode == 1) {
					device_->WriteCommand("!E1\n");
				}
				else if(cTimestampMode == 2) {
					device_->WriteCommand("!E2\n");
				}
				else if(cTimestampMode == 3) {
					device_->WriteCommand("!E3\n");
				}
				else {
					device_->WriteCommand("!E0\n");
				}
				device_->WriteCommand("E+\n");
				running_ = true;
				thread_ = boost::thread(&Edvs::Impl::EventCapture::Run, this);
			}
			void Run() {
				const unsigned char cHighBitMask = 0x80; // 10000000
				const unsigned char cLowerBitsMask = 0x7F; // 01111111
				const unsigned int cNumBytesTimestamp = ((1 <= cTimestampMode && cTimestampMode <= 3) ? (cTimestampMode + 1) : 0);
				const unsigned int cNumBytesPerEvent = 2 + cNumBytesTimestamp;
				unsigned char* buffer = new unsigned char[buffer_size_];
				size_t buffer_offset = 0;
				std::vector<RawEvent>* buff1 = new std::vector<RawEvent>();
				buff1->reserve(buffer_size_ / 2 + 1);
				std::vector<RawEvent>* buff2 = new std::vector<RawEvent>();
				buff2->reserve(buffer_size_ / 2 + 1);
				std::vector<RawEvent>* buffA = buff1;
				std::vector<RawEvent>* buffB = buff2;
				while(running_) {
					size_t bytes_read = 0;
					// if(cEnableFillRead) {
					// 	while(bytes_read < buffer_size_) {
					// 		size_t n = device_->ReadBinaryData(buffer_size_ - bytes_read, (char*)buffer + bytes_read);
					// 		bytes_read += n;
					// 		if(n == 0) {
					// 			break;
					// 		}
					// 	}
					// } else {
						bytes_read = device_->ReadBinaryData(buffer_size_, (char*)buffer + buffer_offset) + buffer_offset;
					// }
					buffA->clear();
					size_t i = 0; // index of current byte
					while(i+cNumBytesPerEvent<bytes_read) {
						// get to bytes
						unsigned char a = buffer[i];
						unsigned char b = buffer[i + 1];
						// check for and parse 0yyyyyyy pxxxxxxx
						if(a & cHighBitMask) { // check that the high bit o first byte is 0
							// the serial port missed a byte somewhere ...
							// skip one byte to jump to the next event
							i ++;
							continue;
						}
						// read time
						uint32_t timestamp;
						if(cTimestampMode == 1) {
							timestamp = (buffer[i+2] << 8) | buffer[i+3];
						}
						else if(cTimestampMode == 2) {
							timestamp = (buffer[i+2] << 16) | (buffer[i+3] << 8) | buffer[i+4];
						}
						else if(cTimestampMode == 3) {
							timestamp = (buffer[i+2] << 24) | (buffer[i+3] << 16) | (buffer[i+4] << 8) | buffer[i+5];
						}
						else {
							timestamp = 0;
						}
						// create event
						RawEvent e;
						e.time = timestamp;
						e.x = b & cLowerBitsMask;
						e.y = a & cLowerBitsMask;
						e.parity = (b & cHighBitMask); // converts to bool
						buffA->push_back(e);
						// increment index
						i += cNumBytesPerEvent;
					}
					// i is now the number of processed bytes
					buffer_offset = bytes_read - i;
#ifdef VERBOSE
					if(buffer_offset != 0) {
						std::cout << "offset: " << buffer_offset << std::endl;
					}
#endif
					if(buffer_offset > 0) {
						for(size_t j=0; j<buffer_offset; j++) {
							buffer[j] = buffer[i + j];
						}
					}
#ifdef VERBOSE
					std::cout << events.size();
#endif
					event_callback_(*buffA);
					std::swap(buffA, buffB);
				}
				delete[] buffer;
				delete buff1;
				delete buff2;
			}
			void StopEventCapture() {
				running_ = false;
				thread_.join();
				device_->WriteCommand("E-\n");
			}
		private:
			boost::shared_ptr<Device> device_;
			EventCallbackType event_callback_;
			size_t buffer_size_;
			bool running_;
			boost::thread thread_;
		};
	}

	/** Wrapper for device implementation */
	struct EventCapture
	{
		static const size_t cDefaultBufferSize = 8192;		
		EventCapture() {
		}
		EventCapture(const boost::shared_ptr<Device>& device, EventCallbackType f, size_t buffer_size=cDefaultBufferSize) {
			capture_.reset(new Impl::EventCapture(device, f, buffer_size));
		}		
	private:
		boost::shared_ptr<Impl::EventCapture> capture_;
	};

}

#endif
