#include "WdgtEventViewer.h"
#include <QtGui>
#include <QApplication>
#include <boost/program_options.hpp>

int main(int argc, char *argv[])
{
	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("filename", po::value<std::string>(), "filename for events file")
		("omnirob", "display omnirob 360 deg from 6+1 cameras")
		("play", "start playing immediately")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	QApplication a(argc, argv);
	WdgtEventViewer w;
	w.show();

	if(vm.count("omnirob")) {
		w.enableOmnirobMode();
	}

	if(vm.count("filename")) {
		w.loadEventFile(vm["filename"].as<std::string>());
	}

	if(vm.count("play")) {
		w.play();
	}

	return a.exec();

}
