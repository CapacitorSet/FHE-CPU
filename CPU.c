#include <stdio.h>
#include <malloc.h>
#include "CPU.h"
#include "TFHE.h"

bit_t getNthBitOfChar(int N, char src) {
	return (bit_t) ((src >> N) & 1);
}

bit_t getN_thBit(int N, bit_t *address, unsigned int bitsInAddress, char *offset) {
	if (bitsInAddress == 1)
		return mux(address[0],
		           getNthBitOfChar(N, offset[1]),
		           getNthBitOfChar(N, offset[0]));
	return mux(address[bitsInAddress - 1],
	           getN_thBit(N, address, bitsInAddress - 1, offset + (1 << (bitsInAddress - 1))),
	           getN_thBit(N, address, bitsInAddress - 1, offset));
}

bit_t getN_thOperandBit(int N, bit_t *address, char* memory) {
	return getN_thBit(N, address, PROGRAM_COUNTER_SIZE, memory);
}

CPUState_t doStep(const CPUState_t cpuState) {
	CPUState_t newState;
	// Look ma, I can retrieve bits!
	bit_t address[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char *memory = calloc(65535, 1);
	memory[0] = (char) 0xff;
	printf("Thing is %d\n", getN_thOperandBit(0, address, memory));
}