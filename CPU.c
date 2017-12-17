#include <stdio.h>
#include <malloc.h>
#include "CPU.h"
#include "TFHE.h"

void constant8(bit_t* dst, const uint8_t src) {
	for (int i = 0; i < 8; i++)
		constant(dst[i], (src >> i) & 1);
}
void constant16(bit_t* dst, const uint16_t src) {
	for (int i = 0; i < 16; i++)
		constant(dst[i], (src >> i) & 1);
}

// Operands
bit_t NOP[8];

void initializeOpcodes(void) {
	initialize(NOP, 8);
	constant8(NOP, 0b00001111);
}

void getN_thBit(bit_t ret, uint8_t N, const bit_t* address, uint8_t bitsInAddress, const bit_t *staticOffset, size_t dynamicOffset) {
	if (bitsInAddress == 1) {
		mux(ret, address[0],
		    (staticOffset + 8 * (dynamicOffset + 1))[N],
		    (staticOffset + 8 * (dynamicOffset + 0))[N]);
		return;
	}
	bit_t a = make_bits(1);
	bit_t b = make_bits(1);
	getN_thBit(a, N, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)));
	getN_thBit(b, N, address, bitsInAddress - 1, staticOffset, dynamicOffset);
	mux(ret, address[bitsInAddress - 1], a, b);
	free_bits(a);
	free_bits(b);
}

void getN_thOperandBit(bit_t ret, uint8_t N, bit_t *address, bit_t* memory) {
	getN_thBit(ret, N, address, BITNESS, memory, 0);
}

// Rather inefficient. Avoid when comparing to known constants (eg. opcodes)!
// Requires 8 + 4 + 2 + 1 = 15 logic gates
void compare8(bit_t dst, const bit_t *a, const bit_t *b) {
	bit_t result0 = make_bits(1); nxor(result0, a[0], b[0]);
	bit_t result1 = make_bits(1); nxor(result1, a[1], b[1]);
	bit_t result2 = make_bits(1); nxor(result2, a[2], b[2]);
	bit_t result3 = make_bits(1); nxor(result3, a[3], b[3]);
	bit_t result4 = make_bits(1); nxor(result4, a[4], b[4]);
	bit_t result5 = make_bits(1); nxor(result5, a[5], b[5]);
	bit_t result6 = make_bits(1); nxor(result6, a[6], b[6]);
	bit_t result7 = make_bits(1); nxor(result7, a[7], b[7]);
	bit_t result01 = make_bits(1); and(result01, result0, result1);
	bit_t result23 = make_bits(1); and(result23, result2, result3);
	bit_t result45 = make_bits(1); and(result45, result4, result5);
	bit_t result67 = make_bits(1); and(result67, result6, result7);
	bit_t result0123 = make_bits(1); and(result0123, result01, result23);
	bit_t result4567 = make_bits(1); and(result4567, result45, result67);
	and(dst, result0123, result4567);
	free_bits(result0);
	free_bits(result1);
	free_bits(result2);
	free_bits(result3);
	free_bits(result4);
	free_bits(result5);
	free_bits(result6);
	free_bits(result7);
	free_bits(result01);
	free_bits(result23);
	free_bits(result45);
	free_bits(result67);
	free_bits(result0123);
	free_bits(result4567);
}

// Optimal functions for comparing unknown bits a1, a2 to known bits b1, b2 (embedded in the name of the function)
// Note that we make trivial functions in order to make arity errors readable
#define optimizedCompare_00(dst, a, b) nor(dst, a, b)
#define optimizedCompare_01(dst, a, b) andny(dst, a, b)
#define optimizedCompare_10(dst, a, b) andyn(dst, a, b)
#define optimizedCompare_11(dst, a, b) and(dst, a, b)

// Creates an optimal decoder by simplification
#define newDetector(name, a0, a1, a2, a3, a4, a5, a6, a7) void detect ## name(bit_t ret, const bit_t *operand) { \
	bit_t compare01 = make_bits(1); optimizedCompare_ ## a0 ## a1(compare01, operand[0], operand[1]); \
	bit_t compare23 = make_bits(1); optimizedCompare_ ## a2 ## a3(compare23, operand[2], operand[3]); \
	bit_t compare45 = make_bits(1); optimizedCompare_ ## a4 ## a5(compare45, operand[4], operand[5]); \
	bit_t compare67 = make_bits(1); optimizedCompare_ ## a6 ## a7(compare67, operand[6], operand[7]); \
	bit_t compare0123 = make_bits(1); and(compare0123, compare01, compare23); \
	bit_t compare4567 = make_bits(1); and(compare4567, compare45, compare67); \
	and(ret, compare0123, compare4567);\
	free(compare01); free(compare23); free(compare45); free(compare67); free(compare0123); free(compare4567); \
}

newDetector(NOP, 0, 0, 0, 0, 1, 1, 1, 1); // creates "bit_t detectNOP(bit_t *operand)"

CPUState_t doStep(const CPUState_t cpuState) {
	CPUState_t newState;
	const bit_t *address = cpuState.programCounter;
	bit_t *memory = cpuState.memory;
	bit_t operand[8]; initialize(operand, 8);
	for (uint8_t i = 0; i < 8; i++)
		getN_thOperandBit(operand[i], i, address, memory);

	bit_t mustIncrementProgramCounterByTwo = 0;

#ifndef WITHOUT_NOP
	// Detect NOP
	bit_t isNOP = make_bits(1);
	detectNOP(isNOP, operand);

	printf("Am I at NOP? %s!", isNOP ? "Yes" : "No");
	// mustIncrementProgramCounterByTwo = or(mustIncrementProgramCounterByTwo, isNOP);
	// tmp = PC + 2 (skip opcode + unused byte)
	// PC = mux(isNOP, tmp, PC)
	free_bits(isNOP);
#endif

	// We don't want to leak memory on every loop, do we?
	free_bits_array(cpuState.programCounter, BITNESS);
	free_bits(cpuState.programCounter);
	free_bits_array(cpuState.memory, (1 << BITNESS) * 8);
	free_bits(cpuState.memory);
}