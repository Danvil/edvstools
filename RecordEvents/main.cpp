#include <Edvs/EventStreamFactory.hpp>
#include <Edvs/EventCapture.hpp>
#include <Edvs/LoadSaveEvents.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <cstdio>

std::vector<Edvs::RawEvent> all_events;
bool verbose = false;

void OnEvent(const std::vector<Edvs::RawEvent>& events)
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
		std::cout << "\tSerial port connection: PORT or PORT?baudrate=BR, e.g. /dev/ttyUSB0 or /dev/ttyUSB0?baudrate=4000000" << std::endl;
		std::cout << "\tRead from event file: /path/to/file" << std::endl;
		return 1;
	}

	std::cout << "Opening event stream ..." << std::endl;
	Edvs::EventStreamHandle stream = Edvs::OpenURI(p_uri);

	std::cout << "Press any key to start recording" << std::endl;
	std::getchar();

	std::cout << "Press any key to stop recording" << std::endl;
	{
		Edvs::EventCaptureHandle capture = Edvs::RunEventCapture(stream, &OnEvent);
		std::getchar();
	}

	std::cout << "Saving " << all_events.size() << " events" << std::endl;
	Edvs::SaveRawEvents(p_fn, all_events);

	return 1;
}
