#ifndef FHE_CPU_CPU_H
#define FHE_CPU_CPU_H

#include "compile_time_settings.h"
#include <stdint.h>
#include "TFHE.h"

typedef struct {
	bits_t programCounter;
	bits_t memory;
	bits_t registers;
} CPUState_t;

void constant16(bit_t dst, uint16_t src);

CPUState_t doStep(const CPUState_t cpuState);

#endif //FHE_CPU_CPU_H
