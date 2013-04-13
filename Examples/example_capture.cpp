#include <Edvs/EventCapture.hpp>
#include <iostream>

void OnEvent(const std::vector<Edvs::Event>& events)
{
	if(events.size() > 0) {
		std::cout << "Time " << events.back().t << ": " << events.size() << " events" << std::endl;
	}
}

int main(int argc, char* argv[])
{
	if(argc != 2) {
		std::cout << "Wrong usage" << std::endl;
		return 0;
	}
	// run event capture
	Edvs::EventStream stream(argv[1]);
	Edvs::EventCapture capture(stream, &OnEvent);
	// run until EOF or Ctrl+C
	while(!stream.eos());
	return 1;
}

