#include <Edvs/EventCapture.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sys/time.h>
#include <stdlib.h>

size_t GetTimeMU() {
	timeval a;
	gettimeofday(&a, NULL);
	return a.tv_sec * 1000000 + a.tv_usec;
}

void MeasureSpeed(const std::vector<Edvs::RawEvent>& events)
{
	// static variables will live over function calls
	static size_t fps_count = 0;
	static size_t fps_time_mus = GetTimeMU();
	static size_t fps_check = 0;
	static float fps = 0.0f;
	static size_t last_max = 0;

	fps_count += events.size();
	if(events.size() > last_max) {
		last_max = events.size();
	}

	if((fps_check++) % 30 == 0) { // do not poll gettimeofday extensively
		size_t dt = GetTimeMU() - fps_time_mus;
		if(dt > 500000) { // write at most every 500 ms
			fps = 1000000.0f * float(fps_count) / float(dt);
			fps_count = 0;
			fps_time_mus += dt;
			std::cout << fps << " events/s; " << "Maximum event count per tick: " << last_max << std::endl;
			last_max = 0;
		}
	}
}

void ShowEvents(const std::vector<Edvs::RawEvent>& events)
{
	std::cout << "Got " << events.size() << " events: ";
	for(std::vector<Edvs::RawEvent>::const_iterator it=events.begin(); it!=events.end(); it++) {
		std::cout << *it << ", ";
	}
	std::cout << std::endl;
}

void OnEvent(const std::vector<Edvs::RawEvent>& events, bool show_events, bool measure_speed)
{
	if(measure_speed) {
		MeasureSpeed(events);
	}
	if(show_events) {
		ShowEvents(events);
	}
}

int main(int argc, char* argv[])
{
	std::string p_device = "";
	std::string p_address = "192.168.201.62:56001";
	std::string p_port = "/dev/ttyUSB0";
	bool p_show_events = false;
	bool p_measure_speed = true;
	Edvs::Baudrate p_baudrate = Edvs::B4000k;

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("device", po::value(&p_device), "edvs connection type: 'net' or 'serial'")
		("address", po::value(&p_address), "network addresse of edvs sensor")
		("port", po::value(&p_port), "serial port")
		("verbose", "report all events on console")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help") || p_device.empty()) {
		std::cout << desc << std::endl;
		return 1;
	}

	if(vm.count("verbose")) {
		p_show_events = true;
		p_measure_speed = false;
	}

	boost::shared_ptr<Edvs::Device> device;
	if(p_device == "net") {
		std::cout << "Connecting via network socket '" << p_address << "'" << std::endl;
		device = Edvs::CreateNetworkDevice(p_address);
	}
	else if(p_device == "serial") {
		std::cout << "Connecting via serial port '" << p_port << "'" << std::endl;
		device = Edvs::CreateSerialDevice(p_baudrate, p_port);
	}
	else {
		std::cerr << "Unknown device type! Supported are 'net' and 'serial'." << std::endl;
		std::exit(0);
	}

	std::cout << "Running event capture ..." << std::endl;
	auto capture = Edvs::RunEventCapture(device,
		boost::bind(OnEvent, _1, p_show_events, p_measure_speed));

	// user input loop - press q to quit
	std::string str;
	while(str != "q") {
		// read command
		std::cin >> str;
		// send command to sensor
		device->WriteCommand(str + "\n");
	}

	return 1;
}
