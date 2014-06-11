#ifndef INCLUDED_EDVS_H
#define INCLUDED_EDVS_H

#include "event.h"
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct edvs_stream_t* edvs_stream_handle;

/** Opens an edvs event stream
 * URI can indicate three types of resources:
 *	Network socket:
 *			URI spec: ${IP}:${PORT}
 *			Example: 192.168.201.51:56000
 *	Serial port:
 *			URI spec: ${PORT}?baudrate=${BAUDRATE}
 *			Example: /dev/ttyUSB0?baudrate=4000000
 *			Example: /dev/ttyUSB0 (assumes baudrate=4000000)
 *	File:
 *			URI spec: ${PATH}?dt=${DT}
 *			Example: /home/david/events?dt=50
 * 			Example: /home/david/events (assumes dt=0)
 * @param uri URI to the event stream
 */
edvs_stream_handle edvs_open(const char* uri);

/** Runs the edvs event stream
 * This starts event transmission
 * @param return 0 on success and negative value on error
 */
int edvs_run(edvs_stream_handle h);

/** Checks if the end of an edvs event stream has been reached
 * @param return 1 if end of stream has been reached
 *			0 if end of stream not reached
 *			negative value on error
 */
int edvs_eos(edvs_stream_handle h);

int edvs_get_master_slave_mode(edvs_stream_handle h);

/** Closes an edvs event stream */
int edvs_close(edvs_stream_handle h);

/** Reads events from an edvs event stream
 * @param h stream handle
 * @param events buffer to store events (must have room for 'n' events)
 * @param n maximal number of events to read
 */
ssize_t edvs_read(edvs_stream_handle h, edvs_event_t* events, size_t n);

ssize_t edvs_read_ext(edvs_stream_handle h, edvs_event_t* events, size_t n, edvs_special_t* special, size_t* ns);

ssize_t edvs_write(edvs_stream_handle h, const char* cmd, size_t n);

/** Reads events from a file */
ssize_t edvs_file_read(FILE* fh, edvs_event_t* events, size_t n);

/** Writes events to a file */
ssize_t edvs_file_write(FILE* fh, const edvs_event_t* events, size_t n);

#ifdef __cplusplus
}
#endif

#endif
