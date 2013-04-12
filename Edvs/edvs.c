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

int edvs_net_open(char* address, int port)
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

/** Reads data from the network socket
 * Reads at most n bytes and stores them in 'data'.
 * Does not block execution.
 * 'data' must have room for at least n bytes
 * Returns the number of bytes actually read or -1 on error.
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

ssize_t edvs_net_write(int sockfd, char* data, size_t n)
{
	ssize_t m = send(sockfd, data, n, 0);
	if(m != n) {
		printf("edvs_net_send: send error %zd\n", m);
	}
	return m;
}

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

int edvs_serial_open(char* path, int baudrate)
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

ssize_t edvs_serial_read(int port, char* data, size_t n)
{
	ssize_t m = read(port, data, n);
	if(m < 0) {
		printf("edvs_serial_read: read error %zd\n", m);
		return -1;
	}
	return m;
}

ssize_t edvs_serial_write(int port, char* data, size_t n)
{
	ssize_t m = write(port, data, n);
	if(m != n) {
		printf("edvs_serial_send: write error %zd\n", m);
		return -1;
	}
	return m;
}

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

typedef enum { EDVS_DEVICE_NET, EDVS_DEVICE_SERIAL } device_type;

typedef struct {
	int handle;
	device_type type;
} edvs_device_t;

ssize_t edvs_device_read(edvs_device_t* dh, char* data, size_t n)
{
	switch(dh->type) {
	case EDVS_DEVICE_NET:
		return edvs_net_read(dh->handle, data, n);
	case EDVS_DEVICE_SERIAL:
		return edvs_serial_read(dh->handle, data, n);
	default:
		return -1;
	}
}

ssize_t edvs_device_write(edvs_device_t* dh, char* data, size_t n)
{
	switch(dh->type) {
	case EDVS_DEVICE_NET:
		return edvs_net_write(dh->handle, data, n);
	case EDVS_DEVICE_SERIAL:
		return edvs_serial_write(dh->handle, data, n);
	default:
		return -1;
	}
}

int edvs_device_close(edvs_device_t* dh)
{
	switch(dh->type) {
	case EDVS_DEVICE_NET:
		return edvs_net_close(dh->handle);
	case EDVS_DEVICE_SERIAL:
		return edvs_serial_close(dh->handle);
	default:
		return -1;
	}
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

typedef struct {
	edvs_device_t* device;
	int timestamp_mode;
	char* buffer;
	size_t length;
	size_t offset;
	uint64_t current_time;
	uint64_t last_timestamp;
} edvs_device_streaming_t;

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
		event_it->parity = (b & cHighBitMask); // converts to bool
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

ssize_t edvs_file_write(FILE* fh, edvs_event_t* events, size_t n)
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
	uint64_t dt;
	edvs_event_t* unprocessed;
	size_t num_max;
	size_t num_curr;
	clock_t start_time;
} edvs_file_streaming_t;

edvs_file_streaming_t* edvs_file_streaming_start(char* filename, uint64_t dt)
{
	edvs_file_streaming_t *s = (edvs_file_streaming_t*)malloc(sizeof(edvs_file_streaming_t));
	if(s == 0) {
		return 0;
	}
	s->fh = fopen(filename, "rb");
	s->dt = dt;
	s->num_max = 1024;
	s->unprocessed = (edvs_event_t*)malloc(s->num_max*sizeof(edvs_event_t));
	s->num_curr = 0;
	s->start_time = clock();
	return s;
}

ssize_t edvs_file_streaming_read(edvs_file_streaming_t* s, edvs_event_t* events, size_t events_max)
{
	// get time
	clock_t current_time = clock();
	uint64_t event_time = ((current_time - s->start_time)*1000000)/CLOCKS_PER_SEC;
	size_t num_total = 0;
	do {
		// read more from stream
		if(s->num_curr == 0) {
			s->num_curr = edvs_file_read(s->fh, s->unprocessed, s->num_max);
			if(s->num_curr < 0) return -1;
		}
		// find first event with time greater equal to desires time
		size_t n = 0;
		while( n < s->num_curr
			&& num_total < events_max
			&& s->unprocessed[n].t < event_time
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
			(void*)(s->unprocessed + n),
			(const void*)s->unprocessed,
			(s->num_curr - n)*sizeof(edvs_event_t));
		s->num_curr -= n;
	}
	while(s->num_curr == 0);
	return num_total;
}

int edvs_file_streaming_stop(edvs_file_streaming_t* s)
{
	fclose(s->fh);
	free(s->unprocessed);
	free(s);
	return 0;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

// #include <string.h>
// #include <stdbool.h>

// typedef struct {
// 	uintptr_t* elements;
// 	size_t num;
// } edvs_stream_list_t;

// edvs_stream_list_t* edvs_stream_list_create()
// {
// 	edvs_stream_list_t* q = (edvs_stream_list_t*)malloc(sizeof(edvs_stream_list_t));
// 	q->num = 16;
// 	q->elements = (uintptr_t*)malloc(q->num*sizeof(uintptr_t));
// 	return q;
// }

// edvs_stream_list_t* s_edvs_stream_list = NULL;

// int edvs_stream_list_add(void* stream)
// {
// 	if(s_edvs_stream_list == NULL) {
// 		s_edvs_stream_list = edvs_stream_list_create();
// 	}
// 	for(size_t i=0; i<s_edvs_stream_list->num; i++) {
// 		if(s_edvs_stream_list->elements[i] == 0) {
// 			s_edvs_stream_list->elements[i] = (uintptr_t)stream;
// 			return 1;
// 		}
// 	}
// 	uintptr_t* sn = (uintptr_t*)malloc((s_edvs_stream_list->num + 1)*sizeof(uintptr_t));
// 	if(sn == NULL) {
// 		return -1;
// 	}
// 	memcpy(sn, s_edvs_stream_list->elements, s_edvs_stream_list->num*sizeof(uintptr_t));
// 	free(s_edvs_stream_list->elements);
// 	s_edvs_stream_list->elements = sn;
// 	s_edvs_stream_list->elements[s_edvs_stream_list->num] = (uintptr_t)stream;
// 	s_edvs_stream_list->num ++;
// 	return 1;
// }

// bool edvs_stream_list_check(void* stream)
// {
// 	if(s_edvs_stream_list == NULL) {
// 		return false;
// 	}
// 	for(size_t i=0; i<s_edvs_stream_list->num; i++) {
// 		if(s_edvs_stream_list->elements[i] == (uintptr_t)stream) {
// 			return true;
// 		}
// 	}
// 	return false;
// }

// int edvs_stream_list_remove(void* stream)
// {
// 	if(s_edvs_stream_list == NULL) {
// 		return -1;
// 	}
// 	for(size_t i=0; i<s_edvs_stream_list->num; i++) {
// 		if(s_edvs_stream_list->elements[i] == (uintptr_t)stream) {
// 			s_edvs_stream_list->elements[i] = 0;
// 			return 1;
// 		}
// 	}
// 	return -1;
// }

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <stdint.h>
#include <string.h>

edvs_stream_t* edvs_open(char* uri)
{
	// check for -> net
	char *pcolon = strstr(uri, ":");
	if(pcolon != NULL) {
		// get ip from uri
		size_t ip_len = pcolon-uri;
		char *ip = (char*)malloc(ip_len+1);
		memcpy(ip, uri, ip_len);
		ip[ip_len] = '\0';
		printf("port=%s", ip);
		// get port from uri
		int port = atoi(pcolon+1);
		printf("port=%d", port);
		// open device
		int dev = edvs_net_open(ip, port);
		if(dev < 0) {
			printf("edvs_open: URI seems to point to a network socket, but connection failed\n");
			return 0;
		}
		free(ip);
		// start streaming
		edvs_device_t* dh = (edvs_device_t*)malloc(sizeof(edvs_device_t));
		dh->type = EDVS_DEVICE_NET;
		dh->handle = dev;
		edvs_device_streaming_t* ds = edvs_device_streaming_start(dh);
		edvs_stream_t* s = (edvs_stream_t*)malloc(sizeof(edvs_stream_t));
		s->type = EDVS_DEVICE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// check for baudrate -> serial
	char *pbaudrate = strstr(uri, "baudrate");
	if(pbaudrate != NULL) {
		// FIXME parse file
		char* port = "/dev/ttyUSB0";
		// FIXME parse get baudrate
		int baudrate = B4000000;
		// open device
		int dev = edvs_serial_open(port, baudrate);
		if(dev < 0) {
			printf("edvs_open: URI seems to point to a serial port, but connection failed\n");
			return 0;
		}
		// start streaming
		edvs_device_t* dh = (edvs_device_t*)malloc(sizeof(edvs_device_t));
		dh->type = EDVS_DEVICE_SERIAL;
		dh->handle = dev;
		edvs_device_streaming_t* ds = edvs_device_streaming_start(dh);
		edvs_stream_t* s = (edvs_stream_t*)malloc(sizeof(edvs_stream_t));
		s->type = EDVS_DEVICE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// else -> file
	{
		// FIXME URI parsing
		edvs_file_streaming_t* ds = edvs_file_streaming_start(uri, 0);
		edvs_stream_t* s = (edvs_stream_t*)malloc(sizeof(edvs_stream_t));
		s->type = EDVS_FILE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// printf("edvs_open: did not recognize URI\n");
	// return 0;
}

int edvs_close(edvs_stream_t* s)
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

ssize_t edvs_read(edvs_stream_t* s, edvs_event_t* events, size_t n)
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

#include <signal.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	if(argc == 1) {
		printf("Usage: cmd ip:port\n");
		return 0;
	}
	edvs_stream_t* s = edvs_open(argv[1]);
	size_t n = 128;
	edvs_event_t* events = (edvs_event_t*)malloc(n*sizeof(edvs_event_t));
	size_t num = 0;
	while(num < 10000) {
		ssize_t m = edvs_read(s, events, n);
		printf("%zd/%d events\n", num, 10000);
		num += m;
	}
	edvs_close(s);
	return 1;
}