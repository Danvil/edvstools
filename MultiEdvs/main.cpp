#include "WdgtEdvsVisual.h"
#include <Edvs/EventStream.hpp>
#include <QtGui>
#include <QApplication>
#include <boost/program_options.hpp>

int main(int argc, char *argv[])
{
	std::string p_uri1 = "/dev/ttyUSB0?baudrate=4000000";
	std::string p_uri2 = "/dev/ttyUSB2?baudrate=4000000";

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("uri1", po::value(&p_uri1)->default_value(p_uri1), "URI to event source 1")
		("uri2", po::value(&p_uri2)->default_value(p_uri2), "URI to event source 2")
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

	Edvs::EventStream stream(std::vector<std::string>{p_uri1, p_uri2});

	QApplication a(argc, argv);
	EdvsVisual w(stream);
	w.show();

	return a.exec();
}
