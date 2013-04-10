#include "WdgtEdvsVisual.h"
#include <Edvs/EventCapture.hpp>
#include <QtGui>
#include <QApplication>
#include <boost/program_options.hpp>

int main(int argc, char *argv[])
{
 	std::string p_device = "";
	std::string p_address = "192.168.201.62:56001";
	std::string p_port = "/dev/ttyUSB0";
	Edvs::Baudrate p_baudrate = Edvs::B4000k;

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("device", po::value(&p_device), "edvs connection type: 'net' or 'serial'")
		("address", po::value(&p_address), "network addresse of edvs sensor")
		("port", po::value(&p_port), "serial port")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help") || p_device.empty()) {
		std::cout << desc << std::endl;
		return 1;
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

	QApplication a(argc, argv);
	EdvsVisual w(device);
	w.show();

	return a.exec();
}
