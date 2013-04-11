#ifndef INCLUDE_EDVS_DEVICEEVENTPARSER_HPP
#define INCLUDE_EDVS_DEVICEEVENTPARSER_HPP

#include "EventStream.hpp"
#include "Device.hpp"
#include <vector>

namespace Edvs
{

	class DeviceEventStream
	: public EventStream
	{
	public:
		DeviceEventStream(const DeviceHandle& device, size_t buffer_size=8192);
		
		~DeviceEventStream();
		
		void read(std::vector<RawEvent>& events);

	private:
		DeviceHandle device_;
		std::vector<unsigned char> buffer_;
		size_t buffer_offset_;
	};

}

#endif
