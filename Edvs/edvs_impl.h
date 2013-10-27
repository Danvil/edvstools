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

/** *** Reading data ***
 * size_t n : maximum number of bytes to read
 * unsigned char* data : this pointer must have room to store 'n' bytes
 * Reading functions return the number of bytes which were acutally read
 * or they return -1 if an error occurred.
 */

/** *** Writing data *** 
 * size_t n : nuber of bytes to write
 * const char* data : data to write
 * The 'data' string should probably be terminated with a '\n' character.
 * Writing functions return 0 if all bytes were successfully written,
 * or they return -1 if an error occurred.
 */


/** Opens a network socket for reading events
 * @param address ip address of network socket
 * @param port port of network socket
 * @return socket handle on success or -1 on failure
 */
int edvs_net_open(const char* address, int port);

/** Reads data from a network socket
 * @param sockfd valid socket handle
 * @param data valid buffer to store data
 * @param n maximum number of bytes to read 
 * @return the number of bytes actually read or -1 on failure
 */
ssize_t edvs_net_read(int sockfd, unsigned char* data, size_t n);

/** Writes data to a network socket */
ssize_t edvs_net_write(int sockfd, const char* data, size_t n);

/** Closes a network socket edvs connection
 * @param sockfd socket handle
 * @return 0 on success or -1 on failure
 */
int edvs_net_close(int sockfd);


/** Opens serial port connection to edvs sensor
 * @param path path to the serial device, e.g. /dev/ttyUSB0
 * @param baudrate connection baudrate speed, e.g. 4000000 for 4 MBit/s
 * @return port handle on success or -1 on failure
 */
int edvs_serial_open(const char* path, int baudrate);

/** Reads data from the serial port
 * @param port valid port handle
 * @param data valid buffer to store data
 * @param n maximum number of bytes to read 
 * @return the number of bytes actually read or -1 on failure
 */
ssize_t edvs_serial_read(int port, unsigned char* data, size_t n);

/** Writes data to the serial port */
ssize_t edvs_serial_write(int port, const char* data, size_t n);

/** Closes a serial port edvs connection
 * @param valid port socket handle
 * @return 0 on success or -1 on failure
 */
int edvs_serial_close(int port);


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

/** Reads data from an edvs device */
ssize_t edvs_device_read(edvs_device_t* dh, unsigned char* data, size_t n);

/** Writes data to an edvs device */
ssize_t edvs_device_write(edvs_device_t* dh, const char* data, size_t n);

/** Closes an edvs device connection */
int edvs_device_close(edvs_device_t* dh);


/** Device streaming parameters and state */
typedef struct {
	edvs_device_t* device;
	int timestamp_mode;
	int use_system_time;
	unsigned char* buffer;
	size_t length;
	size_t offset;
	uint64_t current_time;
	uint64_t last_timestamp;
} edvs_device_streaming_t;

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


/** Starts streaming events from an edvs device */
edvs_device_streaming_t* edvs_device_streaming_start(edvs_device_t* dh);

/** Reads events from an edvs device */
ssize_t edvs_device_streaming_read(edvs_device_streaming_t* s, edvs_event_t* events, size_t n, edvs_special_t* special, size_t* ns);

int edvs_device_streaming_write(edvs_device_streaming_t* s, const char* cmd, size_t n);

/** Stops streaming from an edvs device */
int edvs_device_streaming_stop(edvs_device_streaming_t* s);


/** Reads events from a file (binary) */
ssize_t edvs_file_read(FILE* fh, edvs_event_t* events, size_t n);

/** Writes event to a file (binary) */
ssize_t edvs_file_write(FILE* fh, const edvs_event_t* events, size_t n);


/** Start streaming events from a previously stored binary event file
 * Streaming can work in two modes: realtime and simulation.
 * Realtime (set dt=0): The system clock is used to simulate realtime
 * 			event streaming.
 * Simulation (set dt>0): Each call to 'edvs_file_streaming_read'
 * 			increases an internal clock by 'dt' and reads all
 * 			events which have occurred so far.
 * @param filename
 * @param dt see description
 * @param ts scales time for faster or slower playback
 * @return handle 
 */
edvs_file_streaming_t* edvs_file_streaming_start(const char* filename, uint64_t dt, float ts);

/** Reads events from the event file stream */
ssize_t edvs_file_streaming_read(edvs_file_streaming_t* s, edvs_event_t* events, size_t events_max);

/** Stops streaming from an event file */
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
