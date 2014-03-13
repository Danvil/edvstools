#include "mathlink.h"
#include "edvs.h"
#include <stdlib.h>

edvs_stream_handle s;
const size_t num_max_events = 1024;
edvs_event_t* events = NULL;

extern "C" int addtwo(int i, int j);
extern "C" int edvsOpen(const char* opt);
extern "C" int edvsClose();
extern "C" void edvsGet();

int addtwo(int i, int j)
{
	return i + j;
}

int edvsOpen(const char* opt)
{
	events = (edvs_event_t*)malloc(num_max_events*sizeof(edvs_event_t));
	s = edvs_open(opt);
	return (s == nullptr) ? 0 : 1;
}

int edvsClose()
{
	free(events);
	int ret = edvs_close(s);
	return (ret == 0) ? 1 : 0;
}

void edvsGet()
{
	ssize_t m = edvs_read(s, events, num_max_events);
	long* dims = (long*)malloc(2*sizeof(long));
	dims[0] = m;
	dims[1] = 5;
	int* dat = (int*)malloc(m*5*sizeof(int));
	for(long i=0; i<m; i++) {
		int* p = dat + 5*i;
		edvs_event_t* e = events + i;
		p[0] = e->x;
		p[1] = e->y;
		p[2] = e->parity;
		p[3] = e->t & 0x7FFFFFFF;
		p[4] = (e->t >> 31) & 0x7FFFFFFF; // FIXME highest two bits are hopefully 0...
	}
	MLPutIntegerArray(stdlink, dat, dims, NULL, 2);
	free(dat);
	free(dims);
}

int main(int argc, char* argv[])
{
	return MLMain(argc, argv);
}

