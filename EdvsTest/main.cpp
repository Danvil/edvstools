#include "Edvs.h"
#include <iostream>

/** This function is called whenever new events arrive */
void OnEvent(const std::vector<Edvs::RawEvent>& events)
{
	std::cout << "Got " << events.size() << " events: ";
	for(std::vector<Edvs::RawEvent>::const_iterator it=events.begin(); it!=events.end(); it++) {
		std::cout << *it << ", ";
	}
	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	bool p_use_net = true;

	boost::shared_ptr<Edvs::Device> device;

	if(p_use_net)
	{
		const char* cDeviceName = "192.168.201.62:56001";
		std::cout << "opening device at " << cDeviceName << std::endl;
		device = Edvs::create_network_device(cDeviceName);
	}
	else
	{
		const char* cDeviceName = "/dev/ttyUSB1";
		std::cout << "opening device at " << cDeviceName << std::endl;
		device = Edvs::create_serial_device(Edvs::B4000k, cDeviceName);
	}

	std::cout << "start capturing" << std::endl;
	Edvs::EventCapture capture(device, OnEvent);

	// user input loop - press q to quit
	std::string str;
	while(str != "q") {
		// read command
		std::cin >> str;
		// send command to sensor
		device->WriteCommand(str + "\n");
	}

	return 0;
}
