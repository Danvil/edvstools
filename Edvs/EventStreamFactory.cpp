#include "EventStreamFactory.hpp"
#include "DeviceEventStream.hpp"
#include "SerialPort.hpp"
#include "NetworkSocket.hpp"
#include "FileEventStream.hpp"
#include <boost/algorithm/string.hpp>

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

	EventStreamHandle OpenSerialStream(Baudrate br, std::string port)
	{
		return EventStreamHandle(new DeviceEventStream(
			OpenSerialDevice(br, port)));
	}

	EventStreamHandle OpenNetworkStream(const std::string& address)
	{
		return EventStreamHandle(new DeviceEventStream(
			OpenNetworkDevice(address)));
	}

	EventStreamHandle OpenFileStream(const std::string& filename)
	{
		return EventStreamHandle(new FileEventStream(filename));
	}

	EventStreamHandle OpenURI(const std::string& uri)
	{
		std::cout << "Network: " << uri << std::endl;
		// // check uri
		// // find :
		// bool has_colon = uri.find(':') != uri.end();
		// bool has_point = uri.find('.') != uri.end();
		// bool has_question = uri.find('?') != uri.end();
		// bool has_equal = uri.find('=') != uri.end();
		// bool has_slash = uri.find('/') != uri.end();
		// // guess
		// int[3] order = {0, 1, 2};
		// if(has_colon) {
		// 	order
		// }
		// try network
		try {
			DeviceHandle dh = OpenNetworkDevice(uri);
			return EventStreamHandle(new DeviceEventStream(dh));
		}
		catch(const NetSocket::Exception& e) {
			std::cerr << e.what() << std::endl;
		};
		// try serial
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
			DeviceHandle dh = OpenSerialDevice(br, uri);
			return EventStreamHandle(new DeviceEventStream(dh));
		}
		catch(...) {};
		// last try: filesystem
		try {
			return OpenFileStream(uri);
		}
		catch(...) {
			std::cerr << "Wrong URI!" << std::endl;
			throw 0;
		}
	}

}
