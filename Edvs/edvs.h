#ifndef INCLUDED_EDVS_H
#define INCLUDED_EDVS_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __CPLUSPLUS__
extern "C" {
#endif

typedef enum { EDVS_DEVICE_STREAM, EDVS_FILE_STREAM } stream_type;

typedef struct {
	stream_type type;
	uintptr_t handle;
} edvs_stream_t;

typedef struct {
	uint64_t t;
	uint16_t x, y;
	uint8_t parity;
	uint8_t id;
} edvs_event_t;

edvs_stream_t* edvs_open(char* uri);

int edvs_close(edvs_stream_t* h);

ssize_t edvs_read(edvs_stream_t* h, edvs_event_t* events, size_t n);

ssize_t edvs_file_read(FILE* fh, edvs_event_t* events, size_t n);

ssize_t edvs_file_write(FILE* fh, edvs_event_t* events, size_t n);

#ifdef __CPLUSPLUS__
}
#endif

#endif
