/*
 * main.cpp
 *
 *  Created on: Dez 07, 2012
 *      Author: david
 */

#include <Edvs/Tools/IO.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
	namespace po = boost::program_options;

	std::string p_fn_in;
	std::string p_fn_out;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("in", po::value(&p_fn_in)->required(), "filename for input in matlab format")
		("out", po::value(&p_fn_out)->required(), "filename for output")
	;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
	po::notify(vm);

	if(vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	std::cout << "Loading events from file '" << p_fn_in << "'..." << std::flush;
	std::vector<Edvs::Event> events = Edvs::LoadEventsMatlab(p_fn_in);
	std::cout << " done." << std::endl;

	std::cout << "Saving events to file '" << p_fn_out << "'..." << std::flush;
	Edvs::SaveEvents(events, p_fn_out);
	std::cout << " done." << std::endl;

	return 1;

}
