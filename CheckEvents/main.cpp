/*
 * main.cpp
 *
 *  Created on: Nov 28, 2012
 *      Author: david
 */

#include <Edvs/EventIO.hpp>
#include <boost/program_options.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/p_square_cumulative_distribution.hpp>
#include <boost/range/iterator_range.hpp>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
	namespace po = boost::program_options;
	namespace acc = boost::accumulators;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("filenames", po::value<std::vector<std::string>>(), "filename for event file")
	;

	po::positional_options_description p;
	p.add("filenames", -1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
	po::notify(vm);

	if(vm.count("help") || !vm.count("filenames")) {
		std::cout << desc << std::endl;
		return 1;
	}

	std::vector<std::string> fns = vm["filenames"].as<std::vector<std::string>>();

	for(const std::string& fn : fns) {

		std::cout << "Loading events from file '" << fn << "'..." << std::flush;
		std::vector<Edvs::Event> events = Edvs::LoadEvents(fn);
		std::cout << " done." << std::endl;

		std::cout << "Number of events: " << events.size() << std::endl;
		
		{
			std::cout << "TIMESTAMP ORDER" << std::endl;
			uint64_t last_t = events.front().t;
			for(std::size_t i=0; i<events.size(); i++) {
				uint64_t t = events[i].t;
				if(t < last_t) {
					std::cout << "\tevent " << i << ": " << last_t << " -> " << t << std::endl;
				}
				last_t = t;
			}
			std::cout << "TIMESTAMP ORDER" << std::endl;
		}

		{
			typedef acc::accumulator_set<uint64_t, acc::stats<
					acc::tag::mean
					,acc::tag::variance
					,acc::tag::p_square_cumulative_distribution
					>> accumulator_t;
			accumulator_t a(acc::tag::p_square_cumulative_distribution::num_cells = 5);	

			// fill in data
			for(std::size_t i=1; i<events.size(); i++) {
				uint64_t t1 = events[i-1].t;
				uint64_t t2 = events[i].t;
				if(t1 < t2) {
					a(std::abs(t2 - t1));
				}
			}

			std::cout << "STATISTICS" << std::endl;
			std::cout << "\t" << "mean: " << acc::mean(a) << " µs" << std::endl;
			std::cout << "\t" << "sqrt(variance): " << std::sqrt(acc::variance(a)) << " µs" << std::endl;
			auto histogram = acc::p_square_cumulative_distribution(a);
			std::cout << "\t" << "histogram (20% bins): ";
			for(std::size_t i=1; i<histogram.size(); ++i) {
				std::cout << (int)histogram[i].first << " µs";
				if(i+1 < histogram.size()) {
					std::cout << " - ";
				}
			}
			std::cout << std::endl;
			// for(std::size_t i=0; i<histogram.size(); ++i) {
			// 	std::cout << i << " " << histogram[i].first << " " << histogram[i].second << std::endl;
			// }
			std::cout << "STATISTICS" << std::endl;
		}
	}

	return 1;

}
