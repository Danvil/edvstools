/*
 * main.cpp
 *
 *  Created on: Jul 23, 2012
 *      Author: david
 */

#include <Edvs/Tools/IO.hpp>
#include <boost/program_options.hpp>

int main(int argc, char** argv)
{
	std::string fn_in, fn_out;
	bool omnibot_format;

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("in", po::value<std::string>(&fn_in), "filename of input event file (text format)")
		("out", po::value<std::string>(&fn_out), "filename of output event file (binary format)")
		("omnibot", po::value<bool>(&omnibot_format)->default_value(0), "convert from omniBot7eDVS event log format")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help") || fn_in.empty() || fn_out.empty()) {
		std::cout << desc << std::endl;
		return 1;
	}

	if(omnibot_format){
		//Edvs::SaveEvents(Edvs::LoadEvents(fn_in, Edvs::EventFileFlag::RawWithId), fn_out);
		Edvs::SaveEvents(Edvs::LoadEvents(fn_in, Edvs::EventFileFlags(Edvs::EventFileFlag::RawWithId | Edvs::EventFileFlag::UnwrapTimestamps | Edvs::EventFileFlag::BeginTimeZero)), fn_out);
	} 
	else {
		Edvs::SaveEvents(Edvs::LoadEventsMisc(fn_in, 1), fn_out);
	}
}
