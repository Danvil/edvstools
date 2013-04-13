#include "edvs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits.h>

/** Opens a network socket for reading events
 * @param address ip address of network socket
 * @param port port of network socket
 * @return socket handle on success or -1 on failure
 */
int edvs_net_open(const char* address, int port)
{
	// open socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd <= 0) {
		printf("edvs_net_open: socket error %d\n", sockfd);
		return -1;
	}
	// prepare address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	// inet pton
	int i = inet_pton(AF_INET, address, &addr.sin_addr);
	if(i <= 0) {
		printf("edvs_net_open: inet p_ton error %d\n", i);
		return -1;
	}
	// connect
	if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr))) {
		printf("edvs_net_open: connect error\n");
		return -1;
	}
	// return handle
	return sockfd;
}

/** Reads data from a network socket
 * Reads at most n bytes and stores them in 'data'.
 * Does not block execution.
 * 'data' must have room for at least n bytes
 * @param sockfd socket handle
 * @param data valid buffer to store data
 * @param n maximum number of bytes to read 
 * @return the number of bytes actually read or -1 on failure
 */
ssize_t edvs_net_read(int sockfd, char* data, size_t n)
{
	ssize_t m = recv(sockfd, data, n, 0);
	if(m < 0) {
		printf("edvs_net_read: recv error %zd\n", m);
		return -1;
	}
	return m;
}

/** Writes data to a network socket */
ssize_t edvs_net_write(int sockfd, const char* data, size_t n)
{
	ssize_t m = send(sockfd, data, n, 0);
	if(m != n) {
		printf("edvs_net_send: send error %zd\n", m);
	}
	return m;
}

/** Closes a network socket edvs connection
 * @param sockfd socket handle
 * @return 0 on success or -1 on failure
 */
int edvs_net_close(int sockfd)
{
	int r = shutdown(sockfd, SHUT_RDWR);
	if(r != 0) {
		printf("edvs_net_close: socket shutdown error %d\n", r);
		return -1;
	}
	r = close(sockfd);
	if(r != 0) {
		printf("edvs_net_close: socket close error %d\n", r);
		return -1;
	}
	return 0;				
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <termios.h>
#include <stdio.h>
#include <fcntl.h>

/** Opens serial port connection to edvs sensor */
int edvs_serial_open(const char* path, int baudrate)
{
	int port = open(path, O_RDWR /*| O_NOCTTY/ * | O_NDELAY*/);
	if(port < 0) {
		printf("edvs_serial_open: open error %d\n", port);
		return -1;
	}
//	fcntl(fd, F_SETFL, 0);
	// set baud rates and other options
	struct termios settings;
	if(tcgetattr(port, &settings) != 0) {
		printf("edvs_serial_open: tcgetattr error\n");
		return -1;
	}
	if(cfsetispeed(&settings, baudrate) != 0) {
		printf("edvs_serial_open: cfsetispeed error\n");
		return -1;
	}
	if(cfsetospeed(&settings, baudrate)) {
		printf("edvs_serial_open: cfsetospeed error\n");
		return -1;
	}
	settings.c_cflag = (settings.c_cflag & ~CSIZE) | CS8; // 8 bits
	settings.c_cflag |= CLOCAL | CREAD;
	settings.c_cflag |= CRTSCTS; // use hardware handshaking
	settings.c_iflag = IGNBRK;
	settings.c_oflag = 0;
	settings.c_lflag = 0;
	settings.c_cc[VMIN] = 1; // minimum number of characters to receive before satisfying the read.
	settings.c_cc[VTIME] = 5; // time between characters before satisfying the read.
	// write modified record of parameters to port
	if(tcsetattr(port, TCSANOW, &settings) != 0) {
		printf("edvs_serial_open: tcsetattr error\n");
		return -1;
	}
	return port;
}

/** Reads data from the serial port */
ssize_t edvs_serial_read(int port, char* data, size_t n)
{
	ssize_t m = read(port, data, n);
	if(m < 0) {
		printf("edvs_serial_read: read error %zd\n", m);
		return -1;
	}
	return m;
}

/** Writes data to the serial port */
ssize_t edvs_serial_write(int port, const char* data, size_t n)
{
	ssize_t m = write(port, data, n);
	if(m != n) {
		printf("edvs_serial_send: write error %zd\n", m);
		return -1;
	}
	return m;
}

/** Closes a serial port edvs connection */
int edvs_serial_close(int port)
{
	int r = close(port);
	if(r != 0) {
		printf("edvs_serial_close: close error %d\n", r);
		return -1;
	}
	return r;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

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
ssize_t edvs_device_read(edvs_device_t* dh, char* data, size_t n)
{
	switch(dh->type) {
	case EDVS_NETWORK_DEVICE:
		return edvs_net_read(dh->handle, data, n);
	case EDVS_SERIAL_DEVICE:
		return edvs_serial_read(dh->handle, data, n);
	default:
		return -1;
	}
}

/** Writes data to an edvs device */
ssize_t edvs_device_write(edvs_device_t* dh, char* data, size_t n)
{
	switch(dh->type) {
	case EDVS_NETWORK_DEVICE:
		return edvs_net_write(dh->handle, data, n);
	case EDVS_SERIAL_DEVICE:
		return edvs_serial_write(dh->handle, data, n);
	default:
		return -1;
	}
}

/** Closes an edvs device connection */
int edvs_device_close(edvs_device_t* dh)
{
	switch(dh->type) {
	case EDVS_NETWORK_DEVICE:
		return edvs_net_close(dh->handle);
	case EDVS_SERIAL_DEVICE:
		return edvs_serial_close(dh->handle);
	default:
		return -1;
	}
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

/** Device streaming parameters and state */
typedef struct {
	edvs_device_t* device;
	int timestamp_mode;
	char* buffer;
	size_t length;
	size_t offset;
	uint64_t current_time;
	uint64_t last_timestamp;
} edvs_device_streaming_t;

/** Starts streaming events from an edvs device */
edvs_device_streaming_t* edvs_device_streaming_start(edvs_device_t* dh)
{
	edvs_device_streaming_t *s = (edvs_device_streaming_t*)malloc(sizeof(edvs_device_streaming_t));
	if(s == 0) {
		return 0;
	}
	s->device = dh;
	s->timestamp_mode = 2;
	s->length = 8192;
	s->buffer = (char*)malloc(s->length);
	s->offset = 0;
	s->current_time = 0;
	s->last_timestamp = 0;
	if(s->timestamp_mode == 1) {
		if(edvs_device_write(dh, "!E1\n", 4) != 4)
			return 0;
	}
	else if(s->timestamp_mode == 2) {
		if(edvs_device_write(dh, "!E2\n", 4) != 4)
			return 0;
	}
	else if(s->timestamp_mode == 3) {
		if(edvs_device_write(dh, "!E3\n", 4) != 4)
			return 0;
	}
	else {
		if(edvs_device_write(dh, "!E0\n", 4) != 4)
			return 0;
	}
	if(edvs_device_write(dh, "E+\n", 3) != 3)
		return 0;
	return s;
}

/** Reads events from an edvs device */
ssize_t edvs_device_streaming_read(edvs_device_streaming_t* s, edvs_event_t* events, size_t n)
{
	const int timestamp_mode = s->timestamp_mode;
	const unsigned char cHighBitMask = 0x80; // 10000000
	const unsigned char cLowerBitsMask = 0x7F; // 01111111
	const unsigned int cNumBytesTimestamp = (
		(1 <= timestamp_mode && timestamp_mode <= 3)
		? (timestamp_mode + 1)
		: 0);
	const unsigned int cNumBytesPerEvent = 2 + cNumBytesTimestamp;
	// read bytes
	char* buffer = s->buffer;
	size_t num_bytes_events = n*sizeof(edvs_event_t);
	size_t num_bytes_buffer = s->length - s->offset;
	size_t num_read = (num_bytes_buffer < num_bytes_events ? num_bytes_buffer : num_bytes_events);
	ssize_t bytes_read = edvs_device_read(s->device, buffer + s->offset, num_read);
	if(bytes_read < 0) {
		return bytes_read;
	}
	bytes_read += s->offset;
	// parse events
	ssize_t i = 0; // index of current byte
	edvs_event_t* event_it = events;
	while(i+cNumBytesPerEvent < bytes_read) {
		// get to bytes
		unsigned char a = buffer[i];
		unsigned char b = buffer[i + 1];
		// check for and parse 0yyyyyyy pxxxxxxx
		if(a & cHighBitMask) { // check that the high bit o first byte is 0
			// the serial port missed a byte somewhere ...
			// skip one byte to jump to the next event
			i ++;
			continue;
		}
		// read timestamp
		uint64_t timestamp;
		if(timestamp_mode == 1) {
			timestamp = (buffer[i+2] << 8) | buffer[i+3];
		}
		else if(timestamp_mode == 2) {
			timestamp = (buffer[i+2] << 16) | (buffer[i+3] << 8) | buffer[i+4];
		}
		else if(timestamp_mode == 3) {
			timestamp = (buffer[i+2] << 24) | (buffer[i+3] << 16) | (buffer[i+4] << 8) | buffer[i+5];
		}
		else {
			timestamp = 0;
		}
		// wrap timestamp correctly
		if(s->current_time == 0) {
			s->current_time = 1;
		}
		else {
			if(timestamp > s->last_timestamp) {
				s->current_time += (timestamp - s->last_timestamp);
			}
			else {
				s->current_time += 2 * timestamp;
			}
		}
		s->last_timestamp = timestamp;
		// create event
		event_it->t = s->current_time;
		event_it->x = (uint16_t)(b & cLowerBitsMask);
		event_it->y = (uint16_t)(a & cLowerBitsMask);
		event_it->parity = ((b & cHighBitMask) ? 1 : 0);
		event_it->id = 0;
		event_it++;
		// increment index
		i += cNumBytesPerEvent;
	}
	// i is now the number of processed bytes
	s->offset = bytes_read - i;
	if(s->offset > 0) {
		for(size_t j=0; j<s->offset; j++) {
			buffer[j] = buffer[i + j];
		}
	}
	return event_it - events;
}

/** Stops streaming from an edvs device */
int edvs_device_streaming_stop(edvs_device_streaming_t* s)
{
	if(edvs_device_write(s->device, "E-\n", 3) != 3)
		return -1;
	free(s->buffer);
	free(s);
	return 0;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

ssize_t edvs_file_read(FILE* fh, edvs_event_t* events, size_t n)
{
	return fread((void*)events, sizeof(edvs_event_t), n, fh);
}

ssize_t edvs_file_write(FILE* fh, const edvs_event_t* events, size_t n)
{
	size_t m = fwrite((const void*)events, sizeof(edvs_event_t), n, fh);
	if(m != n) {
		printf("edvs_file_write: could not write to file\n");
		return -1;
	}
	return m;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <time.h>

typedef struct {
	FILE* fh;
	int is_eof;
	uint64_t dt;
	edvs_event_t* unprocessed;
	size_t num_max;
	size_t num_curr;
	clock_t start_time;
	uint64_t current_event_time;
} edvs_file_streaming_t;

/** Start streaming events from a previously stored binary event file
 * Streaming can work in two modes: realtime and simulation.
 * Realtime (set dt=0): The system clock is used to simulate realtime
 * 			event streaming.
 * Simulation (set dt>0): Each call to 'edvs_file_streaming_read'
 * 			increases an internal clock by 'dt' and reads all
 * 			events which have occurred so far.
 * @param filename
 * @param dt
 * @return handle 
 */
edvs_file_streaming_t* edvs_file_streaming_start(const char* filename, uint64_t dt)
{
	edvs_file_streaming_t *s = (edvs_file_streaming_t*)malloc(sizeof(edvs_file_streaming_t));
	if(s == 0) {
		return 0;
	}
	s->fh = fopen(filename, "rb");
	s->is_eof = 0;
	s->dt = dt;
	s->num_max = 1024;
	s->unprocessed = (edvs_event_t*)malloc(s->num_max*sizeof(edvs_event_t));
	s->num_curr = 0;
	if(s->dt == 0) {
		s->start_time = clock();
	}
	s->current_event_time = 0;
	return s;
}

/** Reads events from the event file stream */
ssize_t edvs_file_streaming_read(edvs_file_streaming_t* s, edvs_event_t* events, size_t events_max)
{
	// get time
	if(s->dt == 0) {
		s->current_event_time = ((clock() - s->start_time)*1000000)/CLOCKS_PER_SEC;
	}
	else {
		s->current_event_time += s->dt;
	}
	//printf("t=%ld\n",s->current_event_time);
	size_t num_total = 0;
	do {
		// read more from stream
		if(s->num_curr == 0) {
			s->num_curr = edvs_file_read(s->fh, s->unprocessed, s->num_max);
			if(s->num_curr == 0) s->is_eof = 1;
			return 0;
		}
		// find first event with time greater equal to desires time
		size_t n = 0;
		//printf("ti=%ld\n",s->unprocessed[n].t);
		while( n < s->num_curr
			&& num_total < events_max
			&& s->unprocessed[n].t < s->current_event_time
		) {
			n++;
			num_total++;
		}
		// copy events to output buffer
		memcpy(
			(void*)events,
			(const void*)s->unprocessed,
			n*sizeof(edvs_event_t));
		events += n;
		// move remaining events in unprocessed buffer to start
		memmove(
			(void*)s->unprocessed,
			(const void*)(s->unprocessed + n),
			(s->num_curr - n)*sizeof(edvs_event_t));
		s->num_curr -= n;
	}
	while(s->num_curr == 0);
	//printf("num_total=%zd\n",num_total);
	return num_total;
}

/** Stops streaming from an event file */
int edvs_file_streaming_stop(edvs_file_streaming_t* s)
{
	fclose(s->fh);
	free(s->unprocessed);
	free(s);
	return 0;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <stdint.h>
#include <string.h>

typedef enum { EDVS_DEVICE_STREAM, EDVS_FILE_STREAM } stream_type;

struct edvs_stream_t {
	stream_type type;
	uintptr_t handle;
};

edvs_stream_handle edvs_open(const char* uri)
{
	// check for a ':' -> network socket
	const char *pcolon = strstr(uri, ":");
	if(pcolon != NULL) {
		// get ip from uri
		size_t ip_len = pcolon-uri;
		char *ip = (char*)malloc(ip_len+1);
		memcpy(ip, uri, ip_len);
		ip[ip_len] = '\0';
		// get port from uri
		int port = atoi(pcolon+1);
		// open device
		printf("Opening network socket: ip=%s, port=%d\n", ip, port);
		int dev = edvs_net_open(ip, port);
		if(dev < 0) {
			printf("edvs_open: URI seems to point to a network socket, but connection failed\n");
			return 0;
		}
		free(ip);
		// start streaming
		edvs_device_t* dh = (edvs_device_t*)malloc(sizeof(edvs_device_t));
		dh->type = EDVS_NETWORK_DEVICE;
		dh->handle = dev;
		edvs_device_streaming_t* ds = edvs_device_streaming_start(dh);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_DEVICE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// check for 'baudrate' -> serial
	char *pbaudrate = strstr(uri, "baudrate");
	if(pbaudrate != NULL) {
		// FIXME parse uri for port
		char* port = "/dev/ttyUSB0";
		// FIXME parse uri for baudrate
		int baudrate = B4000000;
		// open device
		printf("Opening serial port: port=%s, baudrate=%d\n", port, baudrate);
		int dev = edvs_serial_open(port, baudrate);
		if(dev < 0) {
			printf("edvs_open: URI seems to point to a serial port, but connection failed\n");
			return 0;
		}
		// start streaming
		edvs_device_t* dh = (edvs_device_t*)malloc(sizeof(edvs_device_t));
		dh->type = EDVS_SERIAL_DEVICE;
		dh->handle = dev;
		edvs_device_streaming_t* ds = edvs_device_streaming_start(dh);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_DEVICE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// else -> file
	{
		// FIXME correct URI parsing
		// FIXME parse dt
		uint64_t dt = 0;
		printf("Opening event file '%s', using dt=%lu\n", uri, dt);
		edvs_file_streaming_t* ds = edvs_file_streaming_start(uri, dt);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_FILE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// printf("edvs_open: did not recognize URI\n");
	// return 0;
}

int edvs_close(edvs_stream_handle s)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		edvs_device_t* dh = ds->device;
		edvs_device_streaming_stop(ds);
		edvs_device_close(dh);
		free(s);
		return 0;
	}
	if(s->type == EDVS_FILE_STREAM) {
		edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
		edvs_file_streaming_stop(ds);
		free(s);
		return 0;
	}
	printf("edvs_close: unknown stream type\n");
	return -1;
}

int edvs_eos(edvs_stream_handle s)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		// TODO can device streams reach end of stream?
		return 0;
	}
	if(s->type == EDVS_FILE_STREAM) {
		edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
		return ds->is_eof;
	}
	printf("edvs_eos: unknown stream type\n");
	return -1;
}

ssize_t edvs_read(edvs_stream_handle s, edvs_event_t* events, size_t n)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		return edvs_device_streaming_read(ds, events, n);
	}
	if(s->type == EDVS_FILE_STREAM) {
		edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
		return edvs_file_streaming_read(ds, events, n);
	}
	printf("edvs_read: unknown stream type\n");
	return -1;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

// #include <signal.h>
// #include <stdio.h>

// int main(int argc, char** argv)
// {
// 	if(argc != 2) {
// 		printf("Wrong usage\n");
// 		return EXIT_FAILURE;
// 	}
// 	edvs_stream_handle s = edvs_open(argv[1]);
// 	size_t n = 128;
// 	edvs_event_t* events = (edvs_event_t*)malloc(n*sizeof(edvs_event_t));
// 	size_t num = 0;
// 	while(num < 10000) {
// 		ssize_t m = edvs_read(s, events, n);
// 		printf("%zd/%d events\n", num, 10000);
// 		num += m;
// 	}
// 	edvs_close(s);
// 	free(events);
// 	return EXIT_SUCCESS;
// }

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //
