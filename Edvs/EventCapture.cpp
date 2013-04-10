#include "EventCapture.hpp"
#include "EventCaptureImpl.hpp"
#include "SerialPort.hpp"
#include "NetworkSocket.hpp"

namespace Edvs
{
	boost::shared_ptr<Device> CreateSerialDevice(Baudrate br, std::string port)
	{
		return boost::shared_ptr<Device>(new SerialPort(br, port));
	}

	boost::shared_ptr<Device> CreateNetworkDevice(const std::string& address)
	{
		return boost::shared_ptr<Device>(new NetSocket(address));
	}

	EventCapture::EventCapture(const boost::shared_ptr<Device>& device, EventCallbackType f, size_t buffer_size)
	{
		capture_.reset(new EventCaptureImpl(device, f, buffer_size));
	}
}
