#include "compile_time_settings.h"
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include "CPU.h"

void constant16(bit_t dst, const uint16_t src) {
	for (int i = 0; i < 16; i++)
		constant(&dst[i], (src >> (15 - i)) & 1);
}

/* Puts into ret the Nth bit of a variable at a given `address` into a memory `staticOffset`
 */
void getN_thBit(bit_t ret, uint8_t N, const bit_t address, uint8_t bitsInAddress, const bit_t staticOffset, size_t dynamicOffset) {
	if (bitsInAddress == 1) {
		assert((8*(dynamicOffset+1) + N) < MEMSIZE); // Assert that the read won't be out of bounds
		mux(ret, &address[0],
		    &staticOffset[8 * (dynamicOffset + 1) + N],
		    &staticOffset[8 * (dynamicOffset    ) + N]);
		return;
	}
#if DEBUG
	// Avoids branching, doing simple recursion instead
	int bit = bootsSymDecrypt(&address[bitsInAddress - 1], secret_key);
	if (bit) {
		getN_thBit(ret, N, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)));
	} else {
		getN_thBit(ret, N, address, bitsInAddress - 1, staticOffset, dynamicOffset);
	}
#else
	// Would branching result in an offset so high it will read out of bounds?
	/* This can naturally happen if an instruction is reading one word ahead (eg.
	 * to read the argument). getN_thbit will scan the entire memory, and when
	 * it scans the last word, the branch "read one word ahead" will overflow.
	 * As a consequence of this, if the machine intentionally reads out of bounds
	 * it will read zeros rather than segfaulting - but you would never want to
	 * read past the memory size, right?
	 */
	int willBranchOverflow = (N + 8 * (1 + dynamicOffset + (1 << (bitsInAddress - 1)))) >= MEMSIZE;
	if (willBranchOverflow) {
		// If yes, force the result to stay in bounds: return the lower branch only.
		getN_thBit(ret, N, address, bitsInAddress - 1, staticOffset, dynamicOffset);
		return;
	}
	bit_t a = make_bits(1);
	getN_thBit(a, N, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)));
	bit_t b = make_bits(1);
	getN_thBit(b, N, address, bitsInAddress - 1, staticOffset, dynamicOffset);
	mux(ret, &address[bitsInAddress - 1], a, b);
	free_bits(a);
	free_bits(b);
#endif
}

// Knowing that the operand is at location `address` in the array `memory`, put its Nth bit in `ret`
void getN_thOperandBit(bit_t ret, uint8_t N, bit_t address, bit_t memory) {
	getN_thBit(ret, N, address, BITNESS, memory, 0);
}

// Optimal functions for comparing unknown bits a1, a2 to known bits b1, b2 (embedded in the name of the function)
#define optimizedCompare_00 _nor
#define optimizedCompare_01 _andny
#define optimizedCompare_10 _andyn
#define optimizedCompare_11 _and

// Creates an optimal decoder by simplification
#define newDetector(name, a0, a1, a2, a3, a4, a5, a6, a7) void detect ## name(bit_t ret, const bit_t operand) { \
	bit_t compare01 = make_bits(1); optimizedCompare_ ## a0 ## a1(compare01, &operand[0], &operand[1]); \
	bit_t compare23 = make_bits(1); optimizedCompare_ ## a2 ## a3(compare23, &operand[2], &operand[3]); \
	bit_t compare45 = make_bits(1); optimizedCompare_ ## a4 ## a5(compare45, &operand[4], &operand[5]); \
	bit_t compare67 = make_bits(1); optimizedCompare_ ## a6 ## a7(compare67, &operand[6], &operand[7]); \
	bit_t compare0123 = make_bits(1); _and(compare0123, compare01, compare23); \
	bit_t compare4567 = make_bits(1); _and(compare4567, compare45, compare67); \
	_and(ret, compare0123, compare4567);\
	free(compare01); free(compare23); free(compare45); free(compare67); free(compare0123); free(compare4567); \
}

newDetector(NOP, 0, 0, 0, 0, 1, 1, 1, 1); // creates "bit_t detectNOP(bit_t *operand)"
newDetector(NOT, 0, 0, 1, 0, 1, 1, 0, 0);

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
void advancePCByTwo(bit_t outProgCtr, const bit_t inProgCtr, const bit_t mustIncrementPC) {
	constant(&outProgCtr[0], 0);
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
		_xor(&outProgCtr[i], &inProgCtr[i], carry);
		_and(newCarry, &inProgCtr[i], carry);
		copy(carry, newCarry);
	}

	free_bits(carry);
	free_bits(newCarry);
}

CPUState_t doStep(const CPUState_t cpuState) {
	CPUState_t newState;
	newState.programCounter = make_bits(BITNESS);

	bit_t address = cpuState.programCounter;
	bit_t memory = cpuState.memory;
	bit_t operand = make_bits(8);

#if DEBUG
	printf("PC: ");
	for (uint8_t i = BITNESS; i --> 0; )
		printf("%d", bootsSymDecrypt(&address[i], secret_key));
	printf("\n");
	for (uint8_t i = 0; i < 8; i++)
		getN_thOperandBit(&operand[i], i, address, memory);
#endif

	// Utility variables
	bit_t srcReg = make_bits(4), dstReg = make_bits(4); // Used eg. in "NOT $rA $rB"
	bit_t mustIncrementProgramCounterByTwo = make_bits(1);
	constant(mustIncrementProgramCounterByTwo, 0);

#if DEBUG
	printf("Operand: ");
	for (uint8_t i = 0; i < 8; i++)
		printf("%d", bootsSymDecrypt(&operand[i], secret_key));
	printf("\n");
#endif

	bit_t isNOP = make_bits(1);
	detectNOP(isNOP, operand);
#if DEBUG
	printf("Is NOP? %d.\n", bootsSymDecrypt(isNOP, secret_key));
#endif
	inplace_or(mustIncrementProgramCounterByTwo, isNOP);
	free_bits(isNOP);

	bit_t isNOT = make_bits(1);
	detectNOT(isNOT, operand);
#if DEBUG
	printf("Is NOT? %d.\n", bootsSymDecrypt(isNOT, secret_key));
#endif
	// Syntax: NOT $rA $rB
	// $rA <- ~$rB
	{
		bit_t A = make_bits(4);
		for (int i = 0; i < 4; i++) {
			getN_thOperandBit(&A[i], 8 + i, address, memory);
			mux(&dstReg[i], isNOT, &A[i], &dstReg[i]);
		}
		bit_t B = make_bits(4);
		for (int i = 0; i < 4; i++) {
			getN_thOperandBit(&B[i], 8 + 4 + i, address, memory);
			mux(&srcReg[i], isNOT, &B[i], &srcReg[i]);
		}

		inplace_or(mustIncrementProgramCounterByTwo, isNOT);
		free_bits(isNOT);
	}

	// printf("Increment PC: %s\n", (*mustIncrementProgramCounterByTwo) ? "Yes" : "No");

	advancePCByTwo(newState.programCounter, cpuState.programCounter, mustIncrementProgramCounterByTwo);

	free_bits(mustIncrementProgramCounterByTwo);

	// We don't want to leak memory on every loop, do we?
	free_bits_array(cpuState.programCounter, BITNESS);
	newState.memory = cpuState.memory;

	return newState;
}