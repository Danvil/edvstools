/*
 * main.cpp
 *
 *  Created on: Jul 23, 2012
 *      Author: david
 */

#include "LoadSaveEvents.hpp"
#include <Edvs/EventIO.hpp>
#include <boost/program_options.hpp>

int main(int argc, char** argv)
{
	std::string p_in;
	std::string p_in_format = "natural";
	std::string p_out;
	std::string p_out_format = "tsv";

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("in", po::value(&p_in), "filename of input event file (text format)")
		("in-format", po::value(&p_in_format)->default_value(p_in_format), "format of input file")
		("out", po::value(&p_out), "filename of output event file (binary format)")
		("out-format", po::value(&p_out_format)->default_value(p_out_format), "format of output file")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help") || p_in.empty() || p_out.empty()) {
		std::cout << desc << std::endl;
		std::cout << "Supported file formats:" << std::endl;
		std::cout << "\tnatural: binary default file format" << std::endl;
		std::cout << "\tcsv: text comma separated values" << std::endl;
		std::cout << "\ttsv: text tab separated values" << std::endl;
		std::cout << "\tjc: JC file format" << std::endl;
		std::cout << "\told: A deprecated binary file format" << std::endl;
		return 1;
	}

	std::vector<Edvs::Event> events;

	std::cout << "Loading events from file '" << p_in << "'..." << std::flush;
	if(p_in_format == "natural") {
		events = Edvs::LoadEvents(p_in);
	}
	else if(p_in_format == "csv") {
		events = Edvs::LoadEventsTable(p_in, ',');
	}
	else if(p_in_format == "tsv") {
		events = Edvs::LoadEventsTable(p_in, '\t');
	}
	else if(p_in_format == "jc") {
		events = Edvs::LoadEventsJC(p_in, true);
	}
	else if(p_in_format == "old") {
		events = Edvs::LoadEventsOld(p_in, true);
	}
	else {
		std::cerr << "Unsupported input file format!" << std::endl;
	}
	std::cout << " done (" << events.size() << " events)." << std::endl;

	std::cout << "Saving events to file '" << p_out << "'..." << std::flush;
	if(p_out_format == "natural") {
		Edvs::SaveEvents(p_out, events);
	}
	else if(p_out_format == "csv") {
		Edvs::SaveEventsTable(p_out, events, ',');
	}
	else if(p_out_format == "tsv") {
		Edvs::SaveEventsTable(p_out, events, '\t');
	}
	else {
		std::cerr << "Unsupported output file format!" << std::endl;
	}
	std::cout << " done." << std::endl;

	return 1;
}
