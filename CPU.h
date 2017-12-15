#ifndef FHE_CPU_CPU_H
#define FHE_CPU_CPU_H

#include <stdint.h>
#include "TFHE.h"

#define BITNESS (16) // 16, 32, 64 bits?

typedef struct {
	bit_t *programCounter;
	bit_t *memory;
} CPUState_t;

CPUState_t doStep(CPUState_t cpuState);

#endif //FHE_CPU_CPU_H
