#include <Edvs/EventStream.hpp>
#include <Edvs/EventCapture.hpp>
#include <Edvs/EventIO.hpp>
#include <boost/program_options.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <cstdio>

std::vector<Edvs::Event> all_events;
bool verbose = false;

void OnEvent(const std::vector<Edvs::Event>& events)
{
	all_events.insert(all_events.end(), events.begin(), events.end());
	if(verbose) {
		std::cout << "Got " << events.size() << " (total=" << all_events.size() << ")" << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::string p_uri = "";
	std::string p_fn = "events";

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("uri", po::value(&p_uri), "URI to event source (use help for more info)")
		("fn", po::value(&p_fn), "Filename of file to save events")
		("verbose", po::value(&verbose), "report all events on console")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		std::cout << desc << std::endl;
		std::cout << "URI type format:" << std::endl;
		std::cout << "\tNetwork socket connection: IP:PORT, e.g. 192.168.201.62:56001" << std::endl;
		std::cout << "\tSerial port connection: PORT?baudrate=BR, e.g. /dev/ttyUSB0?baudrate=4000000" << std::endl;
		std::cout << "\tRead from event file: /path/to/file" << std::endl;
		return 1;
	}

	std::cout << "Opening event stream ..." << std::endl;
	Edvs::EventStream stream(p_uri);

	std::cout << "Press Enter to start recording." << std::endl;
	std::getchar();

	std::cout << "Recording... Press Enter to stop." << std::endl;
	{
		Edvs::EventCapture capture(stream,
			boost::bind(&OnEvent, _1));
		std::getchar();
	}

	std::cout << "Saving " << all_events.size() << " events... " << std::flush;
	Edvs::SaveEvents(p_fn, all_events);
	std::cout << "Done." << std::endl;

	return 1;
}
