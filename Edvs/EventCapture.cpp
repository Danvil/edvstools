#include "EventCapture.hpp"
#include "EventCaptureImpl.hpp"
#include "SerialPort.hpp"
#include "NetworkSocket.hpp"

namespace Edvs
{
	DeviceHandle OpenSerialDevice(Baudrate br, std::string port)
	{
		DeviceHandle h;
		h.device.reset(new SerialPort(br, port));
		return h;
	}

	DeviceHandle OpenNetworkDevice(const std::string& address)
	{
		DeviceHandle h;
		h.device.reset(new NetSocket(address));
		return h;
	}

	void StartEventCapture(DeviceHandle& handle, EventCallbackType callback, size_t buffer_size)
	{
		handle.capture_impl.reset(new EventCaptureImpl(handle.device, callback, buffer_size));
	}
}
