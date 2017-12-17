#ifndef FHE_CPU_CPU_H
#define FHE_CPU_CPU_H

#include <stdint.h>
#include "TFHE.h"

#define BITNESS (4) // 16, 32, 64 bits?

typedef struct {
	bit_t *programCounter;
	bit_t *memory;
} CPUState_t;

void constant8(bit_t *dst, uint8_t src);
void constant16(bit_t *dst, uint16_t src);

void initializeOpcodes(void);

CPUState_t doStep(const CPUState_t cpuState);

#endif //FHE_CPU_CPU_H
