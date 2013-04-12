#ifndef INCLUDED_EDVS_H
#define INCLUDED_EDVS_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __CPLUSPLUS__
extern "C" {
#endif

/** An edvs event */
typedef struct {
	uint64_t t;
	uint16_t x, y;
	uint8_t parity;
	uint8_t id;
} edvs_event_t;

typedef struct edvs_stream_t* edvs_stream_handle;

/** Opens an edvs stream */
edvs_stream_handle edvs_open(char* uri);

/** Closes an edvs stream */
int edvs_close(edvs_stream_handle h);

/** Reads events from an edvs stream
 * @param h stream handle
 * @param events buffer to store events (must have room for 'n' events)
 * @param n maximal number of events to read
 */
ssize_t edvs_read(edvs_stream_handle h, edvs_event_t* events, size_t n);

/** Reads events from a file stream */
ssize_t edvs_file_read(FILE* fh, edvs_event_t* events, size_t n);

/** Writes events to a file stream */
ssize_t edvs_file_write(FILE* fh, edvs_event_t* events, size_t n);

#ifdef __CPLUSPLUS__
}
#endif

#endif
