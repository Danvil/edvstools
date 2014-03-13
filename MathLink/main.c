#include "mathlink.h"
#include "edvs.h"
#include <stdlib.h>

#define NUM_MAX_EVENTS 1024
#define NUM_LINKS 4

typedef struct
{
	edvs_stream_handle s;
	edvs_event_t* events;
} handle_t;

handle_t handles[NUM_LINKS];

// extern "C" int addtwo(int i, int j);
// extern "C" int edvsOpen(const char* opt);
// extern "C" int edvsClose();
// extern "C" void edvsGet();

int addtwo(int i, int j)
{
	return i + j;
}

int edvsOpen(const char* opt)
{
	for(int i=0; i<NUM_LINKS; i++) {
		handle_t* h = &(handles[i]);
		if(h->s == NULL) {
			h->s = edvs_open(opt);
			if(h->s == NULL) {
				return 0;
			}
			return i + 1;
		}
	}
	return 0;
}

int edvsClose(int id)
{
	id --;
	if(!(0 <= id && id < NUM_LINKS)) {
		return 0;
	}
	handle_t* h = &(handles[id]);
	if(h->s == NULL) {
		return 0;
	}
	int ret = edvs_close(h->s);
	h->s = NULL;
	return (ret == 0) ? 1 : 0;
}

// inline int clamp(uint16_t v)
// {
// 	return (v < 0) ? 0 : ((v >= 128) ? 127 : v);
// }

void edvsGet(int id)
{
	ssize_t m = 0;
	handle_t* h = NULL;
	id --;
	if(0 <= id && id < NUM_LINKS) {
		h = &(handles[id]);
		m = edvs_read(h->s, h->events, NUM_MAX_EVENTS);
	}
	// send to Mathematica
	long* dims = (long*)malloc(2*sizeof(long));
	dims[0] = m;
	dims[1] = 5;
	int* dat = (int*)malloc(m*5*sizeof(int));
	for(long i=0; i<m; i++) {
		edvs_event_t* e = h->events + i;
		int* p = dat + 5*i;
		p[0] = e->x;
		p[1] = e->y;
		p[2] = (e->parity == 0) ? -1 : +1;
		p[3] = e->t & 0x7FFFFFFF;
		p[4] = (e->t >> 31) & 0x7FFFFFFF; // FIXME highest two bits are hopefully 0...
	}
	MLPutIntegerArray(stdlink, dat, dims, NULL, 2);
	free(dat);
	free(dims);
}

int main(int argc, char* argv[])
{
	for(int i=0; i<NUM_LINKS; i++) {
		handle_t* h = &(handles[i]);
		h->s = NULL;
		h->events = (edvs_event_t*)malloc(NUM_MAX_EVENTS*sizeof(edvs_event_t));
	}

	return MLMain(argc, argv);
}

