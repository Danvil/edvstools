#ifndef INCLUDE_EDVS_EVENTCAPTUREIMPL_HPP_
#define INCLUDE_EDVS_EVENTCAPTUREIMPL_HPP_

#include "EventCaptureImpl.hpp"
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <string>
#include <vector>

//#define VERBOSE

namespace Edvs
{
	const bool cEnableFillRead = false;

	const unsigned int cTimestampMode = 3;
			
	struct EventCaptureImpl
	{
		EventCaptureImpl() {
		}

		EventCaptureImpl(const boost::shared_ptr<Device>& device, EventCallbackType f, size_t buffer_size)
		: device_(device), event_callback_(f), buffer_size_(buffer_size), running_(false) {
			StartEventCapture();
		}

		~EventCaptureImpl() {
			StopEventCapture();
		}

	private:
		void StartEventCapture() {
			if(running_) {
				throw "Already running!";
			}
			// E0: no timestamps, E1: 16 bit timestamps, E2: 24 bit timestamps
			if(cTimestampMode == 1) {
				device_->sendCommand("!E1\n");
			}
			else if(cTimestampMode == 2) {
				device_->sendCommand("!E2\n");
			}
			else if(cTimestampMode == 3) {
				device_->sendCommand("!E3\n");
			}
			else {
				device_->sendCommand("!E0\n");
			}
			device_->sendCommand("E+\n");
			running_ = true;
			thread_ = boost::thread(&Edvs::EventCaptureImpl::Run, this);
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
				// 		size_t n = device_->read(buffer_size_ - bytes_read, (char*)buffer + bytes_read);
				// 		bytes_read += n;
				// 		if(n == 0) {
				// 			break;
				// 		}
				// 	}
				// } else {
					bytes_read = device_->read(buffer_size_, (char*)buffer + buffer_offset) + buffer_offset;
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
			device_->sendCommand("E-\n");
		}

	private:
		boost::shared_ptr<Device> device_;
		EventCallbackType event_callback_;
		size_t buffer_size_;
		bool running_;
		boost::thread thread_;
	};

}

#endif
