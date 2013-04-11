#include <Edvs/EventStreamFactory.hpp>
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
	std::string p_uri = "";
	bool p_show_events = false;
	bool p_measure_speed = true;

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("uri", po::value(&p_uri), "URI to event source (use help for more info)")
		("verbose", "report all events on console")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		std::cout << desc << std::endl;
		std::cout << "URI type format:" << std::endl;
		std::cout << "\tNetwork socket connection: IP:PORT, e.g. 192.168.201.62:56001" << std::endl;
		std::cout << "\tSerial port connection: PORT or PORT?baudrate=BR, e.g. /dev/ttyUSB0 or /dev/ttyUSB0?baudrate=4000000" << std::endl;
		std::cout << "\tRead from event file: /path/to/file" << std::endl;
		return 1;
	}

	if(vm.count("verbose")) {
		p_show_events = true;
		p_measure_speed = false;
	}

	std::cout << "Opening event stream ..." << std::endl;
	Edvs::EventStreamHandle stream = Edvs::OpenURI(p_uri);

	std::cout << "Running event capture ..." << std::endl;
	Edvs::EventCaptureHandle capture = Edvs::RunEventCapture(stream,
		boost::bind(OnEvent, _1, p_show_events, p_measure_speed));

	// user input loop - press q to quit
	std::string str;
	while(str != "q") {
		// read command
		std::cin >> str;
	}

	return 1;
}
