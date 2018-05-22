#ifndef FHE_CPU_CPU_H
#define FHE_CPU_CPU_H

#include <stdint.h>
#include "TFHE.h"

#define BITNESS (4) // 16, 32, 64 bits?
#define MEMSIZE ((1 << BITNESS) * 8) // Number of addressable bytes, times the number of bits in a byte.

typedef struct {
	bits_t programCounter;
	bits_t memory;
} CPUState_t;

void constant16(bit_t dst, uint16_t src);

CPUState_t doStep(const CPUState_t cpuState);

#endif //FHE_CPU_CPU_H
