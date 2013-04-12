#include "EventStreamFactory.hpp"
#include "SerialPort.hpp"
#include "NetworkSocket.hpp"
#include "RawFileEventStream.hpp"
#include "RawDeviceEventStream.hpp"
#include "RawEventTranslator.hpp"
#include "FileEventStream.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

namespace Edvs
{
	DeviceHandle OpenSerialDevice(Baudrate br, std::string port)
	{
		return DeviceHandle(new SerialPort(br, port));
	}

	DeviceHandle OpenNetworkDevice(const std::string& address)
	{
		return DeviceHandle(new NetSocket(address));
	}

	RawEventStreamHandle OpenRawSerialStream(Baudrate br, std::string port)
	{
		return RawEventStreamHandle(new RawDeviceEventStream(
			OpenSerialDevice(br, port)));
	}

	RawEventStreamHandle OpenRawNetworkStream(const std::string& address)
	{
		return RawEventStreamHandle(new RawDeviceEventStream(
			OpenNetworkDevice(address)));
	}

	RawEventStreamHandle OpenRawFileStream(const std::string& filename)
	{
		return RawEventStreamHandle(new RawFileEventStream(filename));
	}

	EventStreamHandle OpenSerialStream(Baudrate br, std::string port)
	{
		return boost::make_shared<RawEventTranslator>(
			boost::make_shared<RawDeviceEventStream>(
				OpenSerialDevice(br, port)));
	}

	EventStreamHandle OpenNetworkStream(const std::string& address)
	{
		return boost::make_shared<RawEventTranslator>(
			boost::make_shared<RawDeviceEventStream>(
				OpenNetworkDevice(address)));
	}

	EventStreamHandle OpenFileStream(const std::string& filename)
	{
		return EventStreamHandle(new FileEventStream(filename));
	}

	EventStreamHandle OpenURI(const std::string& uri)
	{
		// check uri
		bool has_colon = (uri.find(':') != std::string::npos);
		bool has_question = (uri.find('?') != std::string::npos);
		bool has_equal = (uri.find('=') != std::string::npos);
		// try network
		if(has_colon) {
			try {
				return OpenNetworkStream(uri);
			}
			catch(const NetSocket::Exception& e) {
				std::cerr << "URI is not a network address: " << e.what() << std::endl;
				throw;
			}
		}
		// try serial
		if(has_question && has_equal) {
			try {
				std::vector<std::string> v;
				boost::split(v, uri, boost::is_any_of("?="));
				Baudrate br = B4000k;
				if(v.size() == 3 && v[1] == "baudrate") {
					if(v[2] == "921k") br = B921k;
					if(v[2] == "1000k") br = B1000k;
					if(v[2] == "2000k") br = B2000k;
					if(v[2] == "4000k") br = B4000k;
				}
				return OpenSerialStream(br, v[0]);
			}
			catch(const SerialPort::IOException& e) {
				std::cerr << "URI is not a serial port: " << e.what() << std::endl;
				throw;
			}
		}
		// last try: filesystem
		try {
			return OpenFileStream(uri);
		}
		catch(...) {
			std::cerr << "URI is not a file!" << std::endl;
			throw;
		}
	}

}
