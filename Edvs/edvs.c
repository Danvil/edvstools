#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "edvs.h"
#include "edvs_impl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define EDVS_LOG_MESSAGE
#define EDVS_LOG_VERBOSE
// #define EDVS_LOG_ULTRA

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits.h>

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

ssize_t edvs_net_read(int sockfd, unsigned char* data, size_t n)
{
	ssize_t m = recv(sockfd, data, n, 0);
	if(m < 0) {
		printf("edvs_net_read: recv error %zd\n", m);
		return -1;
	}
	return m;
}

ssize_t edvs_net_write(int sockfd, const char* data, size_t n)
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

int edvs_serial_open(const char* path, int baudrate)
{
	int baudrate_enum;
	switch(baudrate) {
		case 2000000: baudrate_enum = B2000000; break;
		case 4000000: baudrate_enum = B4000000; break;
		default:
			printf("edvs_open: invalid baudrate '%d'!", baudrate);
			return -1;
	}

	int port = open(path, O_RDWR /*| O_NOCTTY/ * | O_NDELAY*/);
	if(port < 0) {
		printf("edvs_serial_open: open error %d\n", port);
		return -1;
	}
	// set baud rates and other options
	struct termios settings;
	if(tcgetattr(port, &settings) != 0) {
		printf("edvs_serial_open: tcgetattr error\n");
		return -1;
	}
	if(cfsetispeed(&settings, baudrate_enum) != 0) {
		printf("edvs_serial_open: cfsetispeed error\n");
		return -1;
	}
	if(cfsetospeed(&settings, baudrate_enum)) {
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

ssize_t edvs_serial_read(int port, unsigned char* data, size_t n)
{
	ssize_t m = read(port, data, n);
	if(m < 0) {
		printf("edvs_serial_read: read error %zd\n", m);
		return -1;
	}
	return m;
}

ssize_t edvs_serial_write(int port, const char* data, size_t n)
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

/** Reads data from an edvs device */
ssize_t edvs_device_read(edvs_device_t* dh, unsigned char* data, size_t n)
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
ssize_t edvs_device_write(edvs_device_t* dh, const char* data, size_t n)
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

int edvs_device_write_str(edvs_device_t* dh, const char* str)
{
	size_t n = strlen(str);
	if(edvs_device_write(dh, str, n) != n) {
		return -1;
	}
	else {
		return 0;
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

uint64_t timestamp_limit(int mode)
{
	switch(mode) {
		default: return 0; // no timestamps
		case 1: return (1ull<<16); // 16 bit
		case 2: return (1ull<<24); // 24 bit
		case 3: return (1ull<<32); // 32 bit
	}
}

uint64_t get_micro_time()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return 1000000ull*(uint64_t)(t.tv_sec) + (uint64_t)(t.tv_nsec)/1000ull;
}

void sleep_ms(unsigned long long milli_secs)
{
	struct timespec ts;
	ts.tv_sec = milli_secs / 1000L;
	ts.tv_nsec = (milli_secs * 1000000L) % 1000000000L;
	nanosleep(&ts, NULL);
}

uint64_t c_uint64_t_max = 0xFFFFFFFFFFFFFFFFL;

edvs_device_streaming_t* edvs_device_streaming_open(edvs_device_t* dh, int device_tsm, int host_tsm, int master_slave_mode)
{
	edvs_device_streaming_t *s = (edvs_device_streaming_t*)malloc(sizeof(edvs_device_streaming_t));
	if(s == 0) {
		return 0;
	}
	s->device = dh;
	s->device_timestamp_mode = device_tsm;
	s->host_timestamp_mode = host_tsm;
	s->master_slave_mode = master_slave_mode;
	s->length = 8192;
	s->buffer = (unsigned char*)malloc(s->length);
	s->offset = 0;
//	s->current_time = 0;
//	s->last_timestamp = timestamp_limit(s->device_timestamp_mode);
	s->ts_last_device = c_uint64_t_max;
	s->ts_last_host = s->ts_last_device;
	s->systime_offset = 0;
	// // reset device
	if(edvs_device_write_str(dh, "R\n") != 0)
		return 0;
	sleep_ms(200);
	// timestamp mode
	if(s->device_timestamp_mode == 1) {
		if(edvs_device_write_str(dh, "!E1\n") != 0)
			return 0;
	}
	else if(s->device_timestamp_mode == 2) {
		if(edvs_device_write_str(dh, "!E2\n") != 0)
			return 0;
	}
	else if(s->device_timestamp_mode == 3) {
		if(edvs_device_write_str(dh, "!E3\n") != 0)
			return 0;
	}
	else {
		if(edvs_device_write_str(dh, "!E0\n") != 0)
			return 0;
	}
	// master slave
	if(s->master_slave_mode == 1) {
		// master
#ifdef EDVS_LOG_MESSAGE
		printf("Arming master\n");
#endif
		if(edvs_device_write_str(dh, "!ETM0\n") != 0)
			return 0;
	}
	else if(s->master_slave_mode == 2) {
		// slave
#ifdef EDVS_LOG_MESSAGE
		printf("Arming slave\n");
#endif
		if(edvs_device_write_str(dh, "!ETS\n") != 0)
			return 0;
	}
	return s;
}

int flush_device(edvs_device_streaming_t* s)
{
	if(s->device->type == EDVS_SERIAL_DEVICE) {
		int port = s->device->handle;
		// set non-blocking
		int flags = fcntl(port, F_GETFL, 0);
		fcntl(port, F_SETFL, flags | O_NONBLOCK);
		// read everything
		unsigned char buff[1024];
		while(edvs_device_read(s->device, buff, 1024) > 0);
		// set blocking
		fcntl(port, F_SETFL, flags);
		return 0;
	}
	if(s->device->type == EDVS_NETWORK_DEVICE) {
		// FIXME
		printf("ERROR flush for network device not implemented\n");
		return -1;
	}
	printf("edvs_run: unknown stream type\n");
	return -1;
}

// str must be null terminated
int wait_for(edvs_device_streaming_t* s, const unsigned char* str)
{
	if(s->device->type == EDVS_SERIAL_DEVICE) {
		size_t pos = 0;
		while(str[pos] != 0) {
			unsigned char c;
			ssize_t n = edvs_device_read(s->device, &c, 1);
			if(n != 1) {
				printf("ERROR wait_for\n");
				return -1;
			}
			if(c == str[pos]) {
				pos ++;
			}
			else {
				pos = 0;
			}
		}
		return 0;
	}
	if(s->device->type == EDVS_NETWORK_DEVICE) {
		// FIXME
		printf("ERROR flush for network device not implemented\n");
		return -1;
	}
	printf("edvs_run: unknown stream type\n");
	return -1;
}

int edvs_device_streaming_run(edvs_device_streaming_t* s)
{
	s->systime_offset = get_micro_time();
	if(s->master_slave_mode == 0) {
#ifdef EDVS_LOG_MESSAGE
		printf("Running as normal (no master/slave)\n");
#endif
	}
	else if(s->master_slave_mode == 1) {
#ifdef EDVS_LOG_MESSAGE
		printf("Running as master\n");
#endif
		// master is giving the go command
		if(edvs_device_write_str(s->device, "!ETM+\n") != 0)
			return -1;			
	}
	else if(s->master_slave_mode == 2) {
#ifdef EDVS_LOG_MESSAGE
		printf("Running as slave\n");
#endif
	}
	else {
		printf("ERROR in edvs_device_streaming_run: Invalid master/slave mode!\n");
	}
	// starting event transmission
#ifdef EDVS_LOG_MESSAGE
	printf("Starting transmission\n");
#endif
	if(edvs_device_write_str(s->device, "E+\n") != 0)
		return -1;
	// wait until we get E+\n back
	wait_for(s, "E+\n");
	return 0;
}

/** Computes delta time between timestamps considering a possible wrap */
uint64_t timestamp_dt(uint64_t t1, uint64_t t2, uint64_t wrap)
{
	if(t2 >= t1) {
		// no wrap
		return t2 - t1;
	}
	else {
		// wrap (assume 1 wrap)
		return (wrap + t2) - t1;
	}
}

/** Unwraps timestamps the easy way (works good for 3 byte ts, be careful with 2 byte ts) */
void compute_timestamps_incremental(edvs_event_t* begin, size_t n, uint64_t last_device, uint64_t last_host, uint64_t wrap)
{
	edvs_event_t* end = begin + n;
	for(edvs_event_t* events=begin; events!=end; ++events) {
		// current device time (wrapped)
		uint64_t t = events->t;
		// delta time since last
		uint64_t dt = timestamp_dt(last_device, t, wrap);
#ifdef EDVS_LOG_VERBOSE
		if(t < last_device) {
			printf("WRAPPING %zd -> %zd\n", last_device, t);
		}
#endif
		// update timestamp
		last_device = t;
		// update timestamp
		last_host += dt;
		events->t = last_host;
//		printf("%"PRIu64"\t%"PRIu64"\n", t, events->t);
	}
}

uint64_t sum_dt(edvs_event_t* begin, edvs_event_t* end, uint64_t last_device, uint64_t wrap)
{
	uint64_t dtsum_device = 0;
	for(edvs_event_t* events=begin; events!=end; ++events) {
		// current device time (wrapped)
		uint64_t t = events->t;
		// delta time since last
		uint64_t dt = timestamp_dt(last_device, t, wrap);
		// update timestamp
		last_device = t;
		// sum up
		dtsum_device += dt;
	}
	return dtsum_device;
}

/** Unwraps timestamps using some ugly black magic */
void compute_timestamps_systime(edvs_event_t* begin, size_t n, uint64_t last_device, uint64_t last_host, uint64_t wrap, uint64_t systime)
{
	edvs_event_t* end = begin + n;
	// compute the total added delta time for all events (device time)
	uint64_t dtsum_device = sum_dt(begin, end, last_device, wrap);
	// delta time for all events (host time)
	uint64_t dtsum_host = systime - last_host;
//	printf("SUM DEVICE %"PRIu64"\tSUM HOST %"PRIu64"\n", dtsum_device, dtsum_host);
	// problem: delta time for device might differ from delta time for host
	// if dt_D < dt_H: this might have a natural cause, so we do not know if we need to do something
	// if dt_D > dt_H: we have a problem as timestamps would not be ordered. to solve this we scale timestamps
	uint64_t rem = 0;
//	printf("%"PRIu64"\t%"PRIu64"\n", (end - 1)->t, systime);
	uint64_t curr_host = systime;
	uint64_t next_set_host = systime;
	for(edvs_event_t* events=end-2; events>=begin; --events) { // iterate backwards (skip last)
		// current device time (wrapped)
		uint64_t t = events->t;
		// current delta time
		uint64_t dt_device = timestamp_dt(t, (events+1)->t, wrap);
		// rescale
		// rem_i-1 + dtD*sH = dtH*sD + rem_i, 0 <= rem_i < dtH
		uint64_t s = rem + dt_device*dtsum_host;
		rem = s % dtsum_device;
		uint64_t dt_host = (s - rem) / dtsum_device;
		curr_host -= dt_host;
		(events+1)->t = next_set_host; // delay set to not corrupt device timesteps needed for computation
		next_set_host = curr_host;
//		printf("AAA %"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\n", dt_device, dt_host, s, rem);
//		printf("%"PRIu64"\t%"PRIu64"\n", t, curr_host);
	}
	begin->t = next_set_host; // set last here due to delayed set
}

ssize_t edvs_device_streaming_read(edvs_device_streaming_t* s, edvs_event_t* events, size_t n, edvs_special_t* special, size_t* ns)
{
	// realtime timer
	uint64_t system_clock_time = 0;
	if(s->host_timestamp_mode == 2) {
		system_clock_time = get_micro_time();
#ifdef EDVS_LOG_ULTRA
		printf("Time: %ld\n", system_clock_time);
#endif
	}
	// constants
	const int timestamp_mode = (s->device_timestamp_mode > 3) ? 0 : s->device_timestamp_mode;
	const unsigned char cHighBitMask = 0x80; // 10000000
	const unsigned char cLowerBitsMask = 0x7F; // 01111111
	const unsigned int cNumBytesTimestamp = (s->device_timestamp_mode == 0) ? 0 : (s->device_timestamp_mode + 1);
	const uint64_t cTimestampLimit = timestamp_limit(timestamp_mode);
	const unsigned int cNumBytesPerEvent = 2 + cNumBytesTimestamp;
	const unsigned int cNumBytesPerSpecial = 2 + cNumBytesTimestamp + 1 + 16;
	const unsigned int cNumBytesAhead = (cNumBytesPerEvent > cNumBytesPerSpecial) ? cNumBytesPerEvent : cNumBytesPerSpecial;
	// read bytes
	unsigned char* buffer_begin = s->buffer;
	unsigned char* buffer = buffer_begin;
	size_t num_bytes_events = n*cNumBytesPerEvent;
	size_t num_bytes_buffer = s->length - s->offset;
	size_t num_read = (num_bytes_buffer < num_bytes_events ? num_bytes_buffer : num_bytes_events);
	ssize_t bytes_read = edvs_device_read(s->device, buffer + s->offset, num_read);
#ifdef EDVS_LOG_ULTRA
	printf("Read %zd bytes from device\n", bytes_read);
	{
		for(ssize_t i=0; i<bytes_read; i++) {
			printf("%c", *(buffer + s->offset + i));
		}
		printf("\n");
		// char* tmp = malloc(bytes_read + 1);
		// memcpy(buffer + s->offset, tmp, bytes_read);
		// tmp[bytes_read] = '\0';
		// printf("%s\n", tmp);
		// free(tmp);
	}
#endif
	size_t num_special = 0;
	if(bytes_read < 0) {
		return bytes_read;
	}
	bytes_read += s->offset;
	// parse events
	ssize_t i = 0; // index of current byte
	edvs_event_t* event_it = events;
	edvs_special_t* special_it = special;
#ifdef EDVS_LOG_ULTRA
	printf("START\n");
#endif
	while(i+cNumBytesAhead < bytes_read) {
		// break if no more room for special
		if(special != 0 && ns != 0 && num_special >= *ns) {
			break;
		}
		// get two bytes
		unsigned char a = buffer[i];
		unsigned char b = buffer[i + 1];
#ifdef EDVS_LOG_ULTRA
//		printf("e: %d %d\n", a, b);
#endif
		i += 2;
		// check for and parse 1yyyyyyy pxxxxxxx
		if((a & cHighBitMask) == 0) { // check that the high bit of first byte is 1
			// the serial port missed a byte somewhere ...
			// skip one byte to jump to the next event
			printf("Error in high bit! Skipping a byte\n");
			i --;
			continue;
		}
		// check for special data
		size_t special_data_len = 0;
		if(special != 0 && a == 0 && b == 0) {
			// get special data length
			special_data_len = (buffer[i] & 0x0F);
			// HACK assuming special data always sends timestamp!
			if(special_data_len >= cNumBytesTimestamp) {
				special_data_len -= cNumBytesTimestamp;
			}
			else {
				printf("ERROR parsing special data length!\n");
			}
			i ++;
#ifdef EDVS_LOG_ULTRA
//			printf("s: len=%ld\n", special_data_len);
#endif
		}
		// read timestamp
		uint64_t timestamp;
		if(timestamp_mode == 1) {
			timestamp =
				  ((uint64_t)(buffer[i  ]) <<  8)
				|  (uint64_t)(buffer[i+1]);
		}
		else if(timestamp_mode == 2) {
			timestamp =
				  ((uint64_t)(buffer[i  ]) << 16)
				| ((uint64_t)(buffer[i+1]) <<  8)
				|  (uint64_t)(buffer[i+2]);
			// printf("%d %d %d %d %d\n", a, b, buffer[i+0], buffer[i+1], buffer[i+2]);
#ifdef EDVS_LOG_ULTRA
			printf("t: %d %d %d -> %ld\n", buffer[i], buffer[i+1], buffer[i+2], timestamp);
#endif
		}
		else if(timestamp_mode == 3) {
			timestamp =
				  ((uint64_t)(buffer[i  ]) << 24)
				| ((uint64_t)(buffer[i+1]) << 16)
				| ((uint64_t)(buffer[i+2]) <<  8)
				|  (uint64_t)(buffer[i+3]);
		}
		else {
			timestamp = 0;
		}
//		printf("%p %zd\n", s, timestamp);
		// advance byte count
		i += cNumBytesTimestamp;
		// compute event time
// 		if(s->host_timestamp_mode == 2) {
// 			// compute time since last
// 			// FIXME this does not assure that timestamps are increasing!!!
// 			uint64_t dt;
// 			if(timestamp < s->last_timestamp) {
// 				// we have a wrap
// 				dt = cTimestampLimit + timestamp - s->last_timestamp;
// 			}
// 			else {
// 				// we do not have a wrap
// 				// OR long time no event => ignore
// 				dt = s->last_timestamp - timestamp;
// 			}
// 			s->current_time = system_clock_time + dt;
// 			s->last_timestamp = timestamp;
// 			// // OLD
// 			// if(s->last_timestamp == cTimestampLimit) {
// 			// // 	// start event time at zero
// 			// // 	// FIXME this is problematic with multiple event streams
// 			// // 	//       as they will have different offsets
// 			// // 	s->last_timestamp = system_clock_time;
// 			// 	s->last_timestamp = 0; // use system clock time zero point
// 			// }
// 			// s->current_time = system_clock_time;
// 			// s->last_timestamp = system_clock_time;
// 		}
// 		else if(s->host_timestamp_mode == 1) {
// 			if(timestamp_mode != 0) {
// 				if(s->current_time < 8) { // ignore timestamps of first 8 events
// 					s->current_time ++;
// 				}
// 				else {
// 					if(s->last_timestamp != cTimestampLimit) {
// 						// FIXME possible errors for 16 bit timestamps if no event for more than 65 ms
// 						// FIXME possible errors for 24/32 bit timestamps if timestamp is wrong
// 						if(timestamp >= s->last_timestamp) {
// 							s->current_time += (timestamp - s->last_timestamp);
// 						}
// 						else {
// 							// s->current_time += 2 * timestamp;
// 							s->current_time += timestamp + (cTimestampLimit - s->last_timestamp);
// 						}
// 					}
// 				}
// 			}
// //			printf("old=%lu \tnew=%lu \tt=%lu\n", s->last_timestamp, timestamp, s->current_time);
// 			s->last_timestamp = timestamp;
// 		}
// 		else {
// 			s->current_time = timestamp;
// 		}

		if(special != 0 && ns != 0 && a == 0 && b == 0) {
			// create special
			special_it->t = timestamp; // FIXME s->current_time;
			special_it->n = special_data_len;
			// read special data
#ifdef EDVS_LOG_ULTRA
			printf("SPECIAL DATA:");
#endif
			for(size_t k=0; k<special_it->n; k++) {
				special_it->data[k] = buffer[i+k];
#ifdef EDVS_LOG_ULTRA
				printf(" %d", special_it->data[k]);
#endif
			}
#ifdef EDVS_LOG_ULTRA
			printf("\n");
#endif
			i += special_it->n;
			special_it++;
			num_special++;
		}
		else {
			// create event
			event_it->t = timestamp;
			event_it->x = (uint16_t)(b & cLowerBitsMask);
			event_it->y = (uint16_t)(a & cLowerBitsMask);
			event_it->parity = ((b & cHighBitMask) ? 1 : 0);
			event_it->id = 0;
			event_it++;
		}
	}
	// i is now the number of processed bytes
	s->offset = bytes_read - i;
	if(s->offset > 0) {
		for(size_t j=0; j<s->offset; j++) {
			buffer[j] = buffer[i + j];
		}
	}
	// return
	if(ns != 0) {
		if(special != 0) {
			*ns = special_it - special;
		}
		else {
			*ns = 0;
		}
	}
	// number of events
	ssize_t num_events = event_it - events;
#ifdef EDVS_LOG_ULTRA
	printf("Parsed %zd events\n", num_events);
#endif
	// correct timestamps
	if(num_events > 0) {
		uint64_t last_device = s->ts_last_device;
		uint64_t last_host = s->ts_last_host;
		if(s->ts_last_host == c_uint64_t_max) {
			last_device = events->t;
			last_host = 0;
		}
//		printf("LAST DEVICE %"PRIu64"\n", last_device);
//		printf("LAST HOST %"PRIu64"\n", last_host);
		s->ts_last_device = (events + num_events - 1)->t;
		if(s->host_timestamp_mode == 1) {
			compute_timestamps_incremental(events, num_events, last_device, last_host, cTimestampLimit);
		}
		if(s->host_timestamp_mode == 2) {
			compute_timestamps_systime(events, num_events, last_device, last_host, cTimestampLimit, system_clock_time - s->systime_offset);
		}
		s->ts_last_host = (events + num_events - 1)->t;
	}
	return num_events;
}

int edvs_device_streaming_write(edvs_device_streaming_t* s, const char* cmd, size_t n)
{
	if(edvs_device_write(s->device, cmd, n) != n)
		return -1;
	return 0;
}

int edvs_device_streaming_stop(edvs_device_streaming_t* s)
{
	int r = edvs_device_streaming_write(s, "E-\n", 3);
	if(r != 0) return r;
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

edvs_file_streaming_t* edvs_file_streaming_open(const char* filename, uint64_t dt, float ts)
{
	edvs_file_streaming_t *s = (edvs_file_streaming_t*)malloc(sizeof(edvs_file_streaming_t));
	if(s == 0) {
		return 0;
	}
	s->fh = fopen(filename, "rb");
	s->is_eof = 0;
	s->dt = dt;
	s->timescale = ts;
	s->num_max = 1024;
	s->unprocessed = (edvs_event_t*)malloc(s->num_max*sizeof(edvs_event_t));
	s->num_curr = 0;
	s->is_first = 1;
	s->start_time = 0;
	s->start_event_time = 0;
	s->current_event_time = 0;
	return s;
}

int edvs_file_streaming_run(edvs_file_streaming_t* s)
{
	s->start_time = clock();
	return 0;
}

ssize_t edvs_file_streaming_read(edvs_file_streaming_t* s, edvs_event_t* events, size_t events_max)
{
	if(s->is_eof) {
		return 0;
	}
	// get time
	if(s->dt == 0) {
		uint64_t nt = ((clock() - s->start_time)*1000000)/CLOCKS_PER_SEC;
		s->current_event_time = s->start_event_time + (uint64_t)(s->timescale*(float)(nt));
	}
	else {
		s->current_event_time += s->dt;
	}
	size_t num_total = 0;
	do {
		// read more from stream
		if(s->num_curr == 0) {
			s->num_curr = edvs_file_read(s->fh, s->unprocessed, s->num_max);
			if(s->num_curr == 0) {
				s->is_eof = 1;
			}
		}
		if(s->is_first) {
			s->start_event_time = s->unprocessed[0].t;
			s->current_event_time = s->start_event_time;
			s->is_first = 0;
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
	while(s->num_curr == 0 && s->is_eof != 1);
	//printf("num_total=%zd\n",num_total);
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

#include <stdint.h>
#include <string.h>

int get_uri_type(const char* uri)
{
	// check for a ':' -> network socket
	if(strstr(uri, ":") != NULL) {
		return 1;
	}
	// check for 'baudrate' -> serial
	if(strstr(uri, "baudrate") != NULL) {
		return 2;
	}
	// otherwise
	return 3;
}

int parse_uri_net(const char* curi, char** ip, int* port, int* dtsm, int* htsm, int* msmode)
{
	// Example URI:
	//   192.168.201.62:56001?dtsm=1&htsm=1

	// default
	*ip = NULL;
	*port = 0;
	*dtsm = 1;
	*htsm = 0;
	*msmode = 0;
	// local copy of uri
	char* uri = malloc(strlen(curi)+1);
	strcpy(uri, curi);
	// parse ip
	char* sip = strtok(uri, ":");
	if(sip == NULL) {
		return 0;
	}
	*ip = malloc(strlen(sip)+1);
	strcpy(*ip, sip);
	// parse port
	char* sport = strtok(NULL, "?");
	if(sport == NULL) {
		return 0;
	}
	*port = atoi(sport);
	// parse query tokens
	char* token = strtok(NULL, "=");
	while(token != NULL) {
		// find = in token and delete
		char* val = strtok(NULL, "&");
		// check which token we are parsing
		if(strcmp(token,"dtsm")==0) {
			*dtsm = atoi(val);
		}
		else if(strcmp(token,"htsm")==0) {
			*htsm = atof(val);
		}
		else if(strcmp(token,"msmode")==0) {
			*msmode = atoi(val);
		}
		else {
			printf("ERROR in parse_uri_file: Invalid URI token '%s'!\n", token);
			return 0;
		}
		// next token
		token = strtok(NULL, "=");
	}
	return 1;
}

int parse_uri_device(const char* curi, char** name, int* baudrate, int* dtsm, int* htsm, int* msmode)
{
	// Example URI:
	//   /dev/ttyUSB0?baudrate=4000000&dtsm=1&htsm=1

	// default
	*name = NULL;
	*baudrate = 4000000;
	*dtsm = 1;
	*htsm = 0;
	*msmode = 0;
	// local copy of uri
	char* uri = malloc(strlen(curi)+1);
	strcpy(uri, curi);
	// parse name
	char* sname = strtok(uri, "?");
	if(sname == NULL) {
		return 0;
	}
	*name = malloc(strlen(sname)+1);
	strcpy(*name, sname);
	// parse query tokens
	char* token = strtok(NULL, "=");
	while(token != NULL) {
		// find = in token and delete
		char* val = strtok(NULL, "&");
		// check which token we are parsing
		if(strcmp(token,"baudrate")==0) {
			*baudrate = atoi(val);
		}
		else if(strcmp(token,"dtsm")==0) {
			*dtsm = atoi(val);
		}
		else if(strcmp(token,"htsm")==0) {
			*htsm = atof(val);
		}
		else if(strcmp(token,"msmode")==0) {
			*msmode = atoi(val);
		}
		else {
			printf("ERROR in parse_uri_file: Invalid URI token '%s'!\n", token);
			return 0;
		}
		// next token
		token = strtok(NULL, "=");
	}
	return 1;
}

int parse_uri_file(const char* curi, char** fn, uint64_t* dt, float* ts)
{
	// Example URI:
	//   /home/david/data/test.tsv?dt=0&ts=0.1

	// default
	*fn = NULL;
	*dt = 0;
	*ts = 1.0f;
	// local copy of uri
	char* uri = malloc(strlen(curi)+1);
	strcpy(uri, curi);
	// parse name
	char* sfn = strtok(uri, "?");
	if(sfn == NULL) {
		return 0;
	}
	*fn = malloc(strlen(sfn)+1);
	strcpy(*fn, sfn);
	// parse query tokens
	char* token = strtok(NULL, "=");
	while(token != NULL) {
		// find = in token and delete
		char* val = strtok(NULL, "&");
		// check which token we are parsing
		if(strcmp(token,"dt")==0) {
			*dt = atoi(val);
		}
		else if(strcmp(token,"ts")==0) {
			*ts = atof(val);
		}
		else {
			printf("ERROR in parse_uri_file: Invalid URI token '%s'!\n", token);
			return 0;
		}
		// next token
		token = strtok(NULL, "=");
	}
	return 1;
}

edvs_stream_handle edvs_open(const char* uri)
{
	int uri_type = get_uri_type(uri);
	if(uri_type == 1) {
		// parse URI
		char* ip;
		int port, dtsm, htsm, msmode;
		if(parse_uri_net(uri, &ip, &port, &dtsm, &htsm, &msmode) == 0) {
			printf("edvs_open: Failed to parse URI\n");
			free(ip);
			return 0;
		}
		// open device
		printf("Opening network socket: ip=%s, port=%d using device_tsm=%d, host_tsm=%d, master/slave=%d\n", ip, port, dtsm, htsm, msmode);
		int dev = edvs_net_open(ip, port);
		if(dev < 0) {
			free(ip);
			printf("edvs_open: URI seems to point to a network socket, but connection failed\n");
			return 0;
		}
		free(ip);
		// start streaming
		edvs_device_t* dh = (edvs_device_t*)malloc(sizeof(edvs_device_t));
		dh->type = EDVS_NETWORK_DEVICE;
		dh->handle = dev;
		edvs_device_streaming_t* ds = edvs_device_streaming_open(dh, dtsm, htsm, msmode);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_DEVICE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	else if(uri_type == 2) {
		// parse URI
		char* port;
		int baudrate, dtsm, htsm, msmode;
		if(parse_uri_device(uri, &port, &baudrate, &dtsm, &htsm, &msmode) == 0) {
			printf("edvs_open: Failed to parse URI\n");
			free(port);
			return 0;
		}
		// open device
		printf("Opening serial port: port=%s using baudrate=%d, device_tsm=%d, host_tsm=%d, master/slave=%d\n", port, baudrate, dtsm, htsm, msmode);
		int dev = edvs_serial_open(port, baudrate);
		if(dev < 0) {
			free(port);
			printf("edvs_open: URI seems to point to a serial port, but connection failed\n");
			return 0;
		}
		free(port);
		// start streaming
		edvs_device_t* dh = (edvs_device_t*)malloc(sizeof(edvs_device_t));
		dh->type = EDVS_SERIAL_DEVICE;
		dh->handle = dev;
		edvs_device_streaming_t* ds = edvs_device_streaming_open(dh, dtsm, htsm, msmode);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_DEVICE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// else -> file
	else if(uri_type == 3) {
		// parse URI
		char* fn;
		uint64_t dt;
		float ts;
		if(parse_uri_file(uri, &fn, &dt, &ts) == 0) {
			printf("edvs_open: Failed to parse URI\n");
			free(fn);
			return 0;
		}
		// open
		printf("Opening event file '%s' using dt=%lu, ts=%f\n", fn, dt, ts);
		edvs_file_streaming_t* ds = edvs_file_streaming_open(fn, dt, ts);
		free(fn);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_FILE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	else {
		printf("edvs_open: Could not identify URI type (net/device/file)\n");
		return 0;
	}
}

int edvs_run(edvs_stream_handle s)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		return edvs_device_streaming_run(ds);
	}
	if(s->type == EDVS_FILE_STREAM) {
		edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
		return edvs_file_streaming_run(ds);
	}
	printf("edvs_run: unknown stream type\n");
	return -1;
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

int edvs_is_live(edvs_stream_handle s)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		return 1;
	}
	if(s->type == EDVS_FILE_STREAM) {
		return 0;
	}
	printf("edvs_is_live: unknown stream type\n");
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

int edvs_get_master_slave_mode(edvs_stream_handle s)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		return ds->master_slave_mode;
	}
	if(s->type == EDVS_FILE_STREAM) {
		return 0;
	}
	printf("edvs_get_master_slave_mode: unknown stream type\n");
	return -1;
}

ssize_t edvs_read(edvs_stream_handle s, edvs_event_t* events, size_t n)
{
	return edvs_read_ext(s, events, n, 0, 0);
}

ssize_t edvs_read_ext(edvs_stream_handle s, edvs_event_t* events, size_t n, edvs_special_t* special, size_t* ns)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		return edvs_device_streaming_read(ds, events, n, special, ns);
	}
	if(s->type == EDVS_FILE_STREAM) {
		edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
		if(ns != 0) {
			*ns = 0;
		}
		return edvs_file_streaming_read(ds, events, n);
	}
	printf("edvs_read: unknown stream type\n");
	return -1;
}

ssize_t edvs_write(edvs_stream_handle s, const char* cmd, size_t n)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		return edvs_device_streaming_write(ds, cmd, n);
	}
	if(s->type == EDVS_FILE_STREAM) {
		printf("edvs_write: ERROR can not write to file stream!\n");
		return -1;
	}
	printf("edvs_write: unknown stream type\n");
	return -1;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //
