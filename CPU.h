#ifndef FHE_CPU_CPU_H
#define FHE_CPU_CPU_H

#include <stdint.h>

typedef uint16_t programCounter_t;

#define PROGRAM_COUNTER_SIZE (16)

typedef struct {
	programCounter_t programCounter;
	char *memory;
} CPUState_t;

CPUState_t doStep(CPUState_t cpuState);

#endif //FHE_CPU_CPU_H
