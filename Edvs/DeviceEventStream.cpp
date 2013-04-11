#ifndef INCLUDE_EDVS_DEVICEEVENTPARSER_HPP_
#define INCLUDE_EDVS_DEVICEEVENTPARSER_HPP_

#include "DeviceEventStream.hpp"
#include <iostream>

//#define VERBOSE

namespace Edvs
{
	const bool cEnableFillRead = false;

	const unsigned int cTimestampMode = 2;
			
	DeviceEventStream::DeviceEventStream(const DeviceHandle& device, size_t buffer_size)
	: device_(device)
	{
		buffer_.resize(buffer_size);
		buffer_offset_ = 0;
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
	}

	DeviceEventStream::~DeviceEventStream()
	{
		device_->sendCommand("E-\n");
	}

	void DeviceEventStream::read(std::vector<RawEvent>& events)
	{
		const unsigned char cHighBitMask = 0x80; // 10000000
		const unsigned char cLowerBitsMask = 0x7F; // 01111111
		const unsigned int cNumBytesTimestamp = ((1 <= cTimestampMode && cTimestampMode <= 3) ? (cTimestampMode + 1) : 0);
		const unsigned int cNumBytesPerEvent = 2 + cNumBytesTimestamp;
		size_t bytes_read = device_->read(buffer_.size(), (char*)buffer_.data() + buffer_offset_) + buffer_offset_;
		events.clear();
		events.reserve(bytes_read/cNumBytesPerEvent);
		size_t i = 0; // index of current byte
		while(i+cNumBytesPerEvent<bytes_read) {
			// get to bytes
			unsigned char a = buffer_[i];
			unsigned char b = buffer_[i + 1];
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
				timestamp = (buffer_[i+2] << 8) | buffer_[i+3];
			}
			else if(cTimestampMode == 2) {
				timestamp = (buffer_[i+2] << 16) | (buffer_[i+3] << 8) | buffer_[i+4];
			}
			else if(cTimestampMode == 3) {
				timestamp = (buffer_[i+2] << 24) | (buffer_[i+3] << 16) | (buffer_[i+4] << 8) | buffer_[i+5];
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
			events.push_back(e);
			// increment index
			i += cNumBytesPerEvent;
		}
		// i is now the number of processed bytes
		buffer_offset_ = bytes_read - i;
#ifdef VERBOSE
		if(buffer_offset_ != 0) {
			std::cout << "offset: " << buffer_offset_ << std::endl;
		}
#endif
		if(buffer_offset_ > 0) {
			for(size_t j=0; j<buffer_offset_; j++) {
				buffer_[j] = buffer_[i + j];
			}
		}
#ifdef VERBOSE
		std::cout << events.size();
#endif
	}

}

#endif
