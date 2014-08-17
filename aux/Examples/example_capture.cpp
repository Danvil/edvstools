#include <Edvs/EventStream.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
	// read uri
	if(argc != 2) {
		std::cout << "Wrong usage" << std::endl;
		return 0;
	}
	// open stream
	std::shared_ptr<Edvs::IEventStream> stream = Edvs::OpenEventStream(argv[1]);
	// capture events (run until EOF or Ctrl+C)
	while(!stream->eos()) {	// FIXME make it possible to stop the loop by pressing a button
		// read events from stream
		auto events = stream->read();
		// display message
		if(!events.empty()) {
			std::cout << "Time " << events.back().t << ": " << events.size() << " events" << std::endl;
		}
	}
	return 1;
}

