#include "edvs.h"
#define __USE_POSIX
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

edvs_stream_handle s;
edvs_event_t* events = NULL;

void foo(int signum) {
	fprintf(stderr, "zonk\n");
	edvs_close(s);
	free(events);
}

int main(int argc, char** argv)
{
	struct sigaction siginthandler;

	siginthandler.sa_handler = foo;
	sigemptyset(&siginthandler.sa_mask);
	siginthandler.sa_flags = 0;

	sigaction(SIGINT, &siginthandler, NULL);


	if(argc != 2) {
		printf("Wrong usage\n");
		return EXIT_FAILURE;
	}
	s = edvs_open(argv[1]);
// run event capture
	size_t num_max_events = 1024;
	events = (edvs_event_t*)malloc(num_max_events*sizeof(edvs_event_t));
	while(!edvs_eos(s)) {
		ssize_t m = edvs_read(s, events, num_max_events);
		if(m > 0) {
			printf("Time %lu: %zd events\n", events->t, m);
		}
	}
	return EXIT_SUCCESS;
}
