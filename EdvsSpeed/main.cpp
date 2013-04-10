#include "Edvs.h"
#include <sys/time.h>
#include <stdlib.h>
#include <iostream>

size_t GetTimeMU() {
	timeval a;
	gettimeofday(&a, NULL);
	return a.tv_sec * 1000000 + a.tv_usec;
}

void OnEvent(const std::vector<Edvs::Event>& events)
{
	// static variables will live over function calls
	static size_t fps_count = 0;
	static size_t fps_time_mus = GetTimeMU();
	static size_t fps_check = 0;
	static float fps = 0.0f;
	static size_t last_max = 0;

	fps_count += events.size();
	if(events.size() > last_max) {
		last_max = events.size();
	}

	if((fps_check++) % 30 == 0) { // do not poll gettimeofday extensively
		size_t dt = GetTimeMU() - fps_time_mus;
		if(dt > 500000) { // write at most every 500 ms
			fps = 1000000.0f * float(fps_count) / float(dt);
			fps_count = 0;
			fps_time_mus += dt;
			std::cout << fps << " events/s; " << "Maximum event count per tick: " << last_max << std::endl;
			last_max = 0;
		}
	}
}

int main(int argc, char* argv[])
{
	Edvs::Device device(Edvs::B4000k);
	Edvs::EventCapture capture(device, OnEvent, 1000000);
	std::string str;
	while(true);
	return 0;
}
