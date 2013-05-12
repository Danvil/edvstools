#ifndef INCLUDED_EDVS_IMPL_H
#define INCLUDED_EDVS_IMPL_H

#include "event.h"
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Type of edvs connection */
typedef enum {
	EDVS_NETWORK_DEVICE, // network socket connection
	EDVS_SERIAL_DEVICE // serial port connection
} device_type;

/** Edvs device handle */
typedef struct {
	int handle;
	device_type type;
} edvs_device_t;

int edvs_net_open(const char* address, int port);
ssize_t edvs_net_read(int sockfd, unsigned char* data, size_t n);
ssize_t edvs_net_write(int sockfd, const char* data, size_t n);
int edvs_net_close(int sockfd);

/** Device streaming parameters and state */
typedef struct {
	edvs_device_t* device;
	int timestamp_mode;
	unsigned char* buffer;
	size_t length;
	size_t offset;
	uint64_t current_time;
	uint64_t last_timestamp;
} edvs_device_streaming_t;

ssize_t edvs_device_read(edvs_device_t* dh, unsigned char* data, size_t n);
ssize_t edvs_device_write(edvs_device_t* dh, const char* data, size_t n);
int edvs_device_close(edvs_device_t* dh);

typedef struct {
	FILE* fh;
	int is_eof;
	uint64_t dt; // 0=realtime, else use dt to increase time each call to read
	float timescale; // used to replay slowed down
	edvs_event_t* unprocessed;
	size_t num_max;
	size_t num_curr;
	clock_t start_time;
	uint64_t current_event_time;
} edvs_file_streaming_t;

edvs_file_streaming_t* edvs_file_streaming_start(const char* filename, uint64_t dt, float ts);
ssize_t edvs_file_streaming_read(edvs_file_streaming_t* s, edvs_event_t* events, size_t events_max);
int edvs_file_streaming_stop(edvs_file_streaming_t* s);

typedef enum { EDVS_DEVICE_STREAM, EDVS_FILE_STREAM } stream_type;

struct edvs_stream_t {
	stream_type type;
	uintptr_t handle;
};

#ifdef __cplusplus
}
#endif

#endif
