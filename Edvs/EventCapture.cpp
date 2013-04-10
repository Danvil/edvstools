#include "EventCapture.hpp"
#include "EventCaptureImpl.hpp"
#include "SerialPort.hpp"
#include "NetworkSocket.hpp"

namespace Edvs
{
	DeviceHandle CreateSerialDevice(Baudrate br, std::string port)
	{
		DeviceHandle h;
		h.device.reset(new SerialPort(br, port));
		return h;
	}

	DeviceHandle CreateNetworkDevice(const std::string& address)
	{
		DeviceHandle h;
		h.device.reset(new NetSocket(address));
		return h;
	}

	void RunEventCapture(DeviceHandle& handle, EventCallbackType callback, size_t buffer_size)
	{
		handle.capture_impl.reset(new EventCaptureImpl(handle.device, callback, buffer_size));
	}
}
