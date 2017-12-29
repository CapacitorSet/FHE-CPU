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
		constant(dst[15 - i], (src >> i) & 1); // That's not a typo, 15 is correct.
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

/* The most basic way to do this with FHE would be to create a new variable containing
 * the PC incremented by two, and then the new counter would be
 *
 *     mux(newPC, mustIncrementPC, incrementedPC, oldPC) // i.e. newPC = mustIncrementPC ? incrementedPC : oldPC
 *
 * However, if the PC is N bytes long, that would require 2*N binary gates to do the addition
 * and then N mux gates to do the multiplexing.
 * We can use a clever trick instead: increment by (mustIncrementPC ? 2 : 0). By doing
 * this, it takes us __just__ 2*N gates for the addition!
 */
void advancePCByTwo(bit_t **outProgCtr, const bit_t *inProgCtr, const bit_t mustIncrementPC) {
	constant((*outProgCtr)[0], 0);
	//copy((*outProgCtr)[0], inProgCtr[0]); // Since we're adding two, the first bit is unchanged.
	/* Note that since we're adding 0b0000...00010 to a number, we don't need to implement a
	 * full adder, a half adder will do (the second bit has A = input, B = 1, the others have
	 * A = input, B = carry in)
	 */
	// Sum = A xor B
	// Carry = A and B
	bit_t carry = make_bits(1);
	copy(carry, mustIncrementPC); // This is the core of the no-mux optimization described above.

	bit_t newCarry = make_bits(1);
	for (int i = 1; i < BITNESS; i++) {
		xor((*outProgCtr)[i], inProgCtr[i], carry);
		and(newCarry, inProgCtr[i], carry);
		copy(carry, newCarry);
	}

	free_bits(carry);
	free_bits(newCarry);
}

CPUState_t doStep(const CPUState_t cpuState) {
	CPUState_t newState;
	newState.programCounter = malloc(BITNESS * sizeof(bit_t)); initialize(newState.programCounter, BITNESS);

	const bit_t *address = cpuState.programCounter;
	bit_t *memory = cpuState.memory;
	bit_t operand[8]; initialize(operand, 8);
	for (uint8_t i = 0; i < 8; i++)
		getN_thOperandBit(operand[i], i, address, memory);

	bit_t mustIncrementProgramCounterByTwo = make_bits(1);
	constant(mustIncrementProgramCounterByTwo, 0);

#ifndef WITHOUT_NOP
	// Detect NOP
	bit_t isNOP = make_bits(1);
	detectNOP(isNOP, operand);

	printf("Am I at NOP? %s!\n", (*isNOP) ? "Yes" : "No");
	inplace_or(mustIncrementProgramCounterByTwo, isNOP);
	free_bits(isNOP);
#endif

	printf("Must increment PC? %d.\n", *mustIncrementProgramCounterByTwo);

	advancePCByTwo(&(newState.programCounter), cpuState.programCounter, mustIncrementProgramCounterByTwo);

	free_bits(mustIncrementProgramCounterByTwo);

	// We don't want to leak memory on every loop, do we?
	free_bits_array(cpuState.programCounter, BITNESS);
	free(cpuState.programCounter);
	/* This would be the right thing to do, but also fucking wasteful.
	free_bits_array(cpuState.memory, (1 << BITNESS) * 8);
	free(cpuState.memory);
	*/
	newState.memory = cpuState.memory;

	return newState;
}