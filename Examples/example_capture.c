#include "edvs.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	if(argc != 2) {
		printf("Wrong usage\n");
		return EXIT_FAILURE;
	}
	// run event capture
	edvs_stream_handle s = edvs_open(argv[1]);
	size_t num_max_events = 1024;
	edvs_event_t* events = (edvs_event_t*)malloc(num_max_events*sizeof(edvs_event_t));
	while(!edvs_eos(s)) {
		ssize_t m = edvs_read(s, events, num_max_events);
		if(m > 0) {
			printf("Time %lu: %zd events\n", events->t, m);
		}
	}
	// cleanup
	edvs_close(s);
	free(events);
	return EXIT_SUCCESS;
}
