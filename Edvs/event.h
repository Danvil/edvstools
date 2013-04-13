#ifndef INCLUDED_EDVS_EVENT_H
#define INCLUDED_EDVS_EVENT_H

#include <stdint.h>

#ifdef __CPLUSPLUS__
extern "C" {
#endif

/** An edvs event
 * Struct uses 14 bytes of data.
 */
typedef struct {
	uint64_t t;
	uint16_t x, y;
	uint8_t parity;
	uint8_t id;
} edvs_event_t;

#ifdef __CPLUSPLUS__
}
#endif

#endif
