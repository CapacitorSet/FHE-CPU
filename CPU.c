#include <stdio.h>
#include <malloc.h>
#include "CPU.h"
#include "TFHE.h"

// Operands
const bit_t NOP[8] = {0, 0, 0, 0, 1, 1, 1, 1};

bit_t getN_thBit(uint8_t N, const bit_t *address, uint8_t bitsInAddress, const bit_t *staticOffset, size_t dynamicOffset) {
	if (bitsInAddress == 1)
		return mux(address[0],
		           (staticOffset + 8 * (dynamicOffset + 1))[N],
		           (staticOffset + 8 * (dynamicOffset + 0))[N]);
	return mux(address[bitsInAddress - 1],
	           getN_thBit(N, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1))),
	           getN_thBit(N, address, bitsInAddress - 1, staticOffset, dynamicOffset));
}

bit_t getN_thOperandBit(uint8_t N, bit_t *address, bit_t* memory) {
	return getN_thBit(N, address, BITNESS, memory, 0);
}

// Rather inefficient. Avoid when comparing to known constants (eg. opcodes)!
// Requires 8 + 4 + 2 + 1 = 15 logic gates
bit_t compare8(const bit_t *a, const bit_t *b) {
	bit_t result0 = nxor(a[0], b[0]);
	bit_t result1 = nxor(a[1], b[1]);
	bit_t result2 = nxor(a[2], b[2]);
	bit_t result3 = nxor(a[3], b[3]);
	bit_t result4 = nxor(a[4], b[4]);
	bit_t result5 = nxor(a[5], b[5]);
	bit_t result6 = nxor(a[6], b[6]);
	bit_t result7 = nxor(a[7], b[7]);
	bit_t result01 = and(result0, result1);
	bit_t result23 = and(result2, result3);
	bit_t result45 = and(result4, result5);
	bit_t result67 = and(result6, result7);
	bit_t result0123 = and(result01, result23);
	bit_t result4567 = and(result45, result67);
	return and(result0123, result4567);
}

// Optimal functions for comparing unknown bits a1, a2 to known bits b1, b2 (embedded in the name of the function)
// Note that we make trivial functions in order to make arity errors readable
#define optimizedCompare_00(a, b) nor(a, b)
#define optimizedCompare_01(a, b) andny(a, b)
#define optimizedCompare_10(a, b) andyn(a, b)
#define optimizedCompare_11(a, b) and(a, b)

// Creates an optimal decoder by simplification
#define newDetector(name, a0, a1, a2, a3, a4, a5, a6, a7) bit_t detect ## name(bit_t *operand) { \
	bit_t compare01 = optimizedCompare_ ## a0 ## a1(operand[0], operand[1]); \
	bit_t compare23 = optimizedCompare_ ## a2 ## a3(operand[2], operand[3]); \
	bit_t compare45 = optimizedCompare_ ## a4 ## a5(operand[4], operand[5]); \
	bit_t compare67 = optimizedCompare_ ## a6 ## a7(operand[6], operand[7]); \
	return and(and(compare01, compare23), and(compare45, compare67));\
}

newDetector(NOP, 0, 0, 0, 0, 1, 1, 1, 1); // creates "bit_t detectNOP(bit_t *operand)"

CPUState_t doStep(const CPUState_t cpuState) {
	CPUState_t newState;
	const bit_t *address = cpuState.programCounter;
	bit_t *memory = cpuState.memory;
	bit_t operand[8];
	for (int i = 0; i < 8; i++)
		copy(&operand[i], getN_thOperandBit(i, address, memory));

#ifndef WITHOUT_NOP
	// Detect NOP
	bit_t isNOP = detectNOP(operand);

	printf("Am I at NOP? %s!", isNOP ? "Yes" : "No");
	// tmp = PC + 2 (skip opcode + unused byte)
	// PC = mux(isNOP, tmp, PC)
#endif
}