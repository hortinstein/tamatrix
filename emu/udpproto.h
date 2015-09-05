#ifndef UDPPROTO_H
#define UDPPROTO_H

#include <stdint.h>

#define TAMAUDP_IMAGE 0
#define TAMAUDP_IRSTART 1
#define TAMAUDP_IRDATA 2

typedef struct __attribute__((packed)) {
	uint8_t pixel[32][48];
	uint16_t icons;
} TamaUdpDisplay;

typedef struct __attribute__((packed)) {
	uint16_t dataLen;
	uint8_t data[32];
} TamaIrData;

typedef struct __attribute__((packed)) {
	uint8_t type;
} TamaIrStartData;



typedef struct __attribute__((packed)) {
	uint8_t type;
	union {
		TamaUdpDisplay disp;
		TamaIrData ir;
		TamaIrStartData irs;
	} d;
} TamaUdpData;

#endif