#ifndef INCLUDE_EDVS_RAWDEVICEEVENTPARSER_HPP
#define INCLUDE_EDVS_RAWDEVICEEVENTPARSER_HPP

#include "EventStream.hpp"
#include "Device.hpp"
#include <vector>

namespace Edvs
{

	class RawDeviceEventStream
	: public RawEventStream
	{
	public:
		RawDeviceEventStream(const DeviceHandle& device, size_t buffer_size=8192);
		
		~RawDeviceEventStream();
		
		void read(std::vector<RawEvent>& events);

		bool eof() const { return false; }

	private:
		DeviceHandle device_;
		std::vector<unsigned char> buffer_;
		size_t buffer_offset_;
	};

}

#endif
