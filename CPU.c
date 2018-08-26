#include "compile_time_settings.h"
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <stdbool.h>
#include "CPU.h"
#include "debug.h"
#include "utils.h"

/* Puts into an array staticOffset, at the `address`, at the Nth bit in the word, a bit `src`
 */
void putN_thBit(bit_t src, uint8_t N, uint8_t wordsize, const bit_t address, uint8_t bitsInAddress, const bit_t staticOffset, size_t dynamicOffset, bit_t mask) {
	assert(N < wordsize);
	if (bitsInAddress == 1) {
		assert((8*(dynamicOffset+1) + N) < MEMSIZE); // Assert that the write won't be out of bounds
		bit_t bit;
		bit = &staticOffset[wordsize * (dynamicOffset    ) + N];
		bit_t lowerMask = make_bits(1);
		_andyn(lowerMask, mask, &address[0]);
		mux(bit, lowerMask, src, bit);
		free_bits(lowerMask);

		bit = &staticOffset[wordsize * (dynamicOffset + 1) + N];
		bit_t upperMask = make_bits(1);
		_and(upperMask, mask, &address[0]);
		mux(bit, upperMask, src, bit);
		free_bits(lowerMask);
		return;
	}
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
		putN_thBit(src, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset, mask);
		return;
	}
	bit_t upperMask = make_bits(1);
	_and(upperMask, mask, &address[bitsInAddress - 1]);
#if TRIVIAL_putNth_bit
	if (decrypt(upperMask))
#endif
	putN_thBit(src, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)), upperMask);
	free_bits(upperMask);

	bit_t lowerMask = make_bits(1);
	_andyn(lowerMask, mask, &address[bitsInAddress - 1]);
#if TRIVIAL_putNth_bit
	if (decrypt(lowerMask))
#endif
	putN_thBit(src, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset, lowerMask);
	free_bits(lowerMask);
}

/* Puts into ret the Nth bit of a variable at a given `address` into a memory `staticOffset`
 */
void getN_thBit(bit_t ret, uint8_t N, uint8_t wordsize, const bit_t address, uint8_t bitsInAddress, const bit_t staticOffset, size_t dynamicOffset) {
	/* The assertion should be there, but it's also useful sometimes to override
	 * it (eg. reading operand). Maybe I'll add it in sometime.
	 */
	// assert(N < wordsize);
	if (bitsInAddress == 1) {
		assert((8*(dynamicOffset+1) + N) < MEMSIZE); // Assert that the read won't be out of bounds
		mux(ret, &address[0],
		    &staticOffset[wordsize * (dynamicOffset + 1) + N],
		    &staticOffset[wordsize * (dynamicOffset    ) + N]);
		return;
	}
#if TRIVIAL_getNth_bit
	// Avoids branching, doing simple recursion instead
	int bit = decrypt(&address[bitsInAddress - 1]);
	if (bit) {
		getN_thBit(ret, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)));
	} else {
		getN_thBit(ret, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset);
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
		getN_thBit(ret, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset);
		return;
	}
	bit_t a = make_bits(1);
	getN_thBit(a, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)));
	bit_t b = make_bits(1);
	getN_thBit(b, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset);
	mux(ret, &address[bitsInAddress - 1], a, b);
	free_bits(a);
	free_bits(b);
#endif
}

// Knowing that the operand is at location `address` in the array `memory`, put its Nth bit in `ret`
void getN_thOperandBit(bit_t ret, uint8_t N, bit_t address, bit_t memory) {
	getN_thBit(ret, N, 8, address, BITNESS, memory, 0);
}

optDetector(NOP, 0, 0, 0, 0, 1, 1, 1, 1); // creates "bit_t detectNOP(bit_t *operand)"
optDetector(NOT, 0, 0, 1, 0, 1, 1, 0, 0);

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
	newState.registers = make_bits(NUMREGISTERS * REGISTERSIZE);

	bit_t address = cpuState.programCounter;
	bit_t memory = cpuState.memory;
	bit_t operand = make_bits(8);

#if DEBUG
	printf("PC: ");
	printLE(address, BITNESS);
	printf("\n");
#endif

	// Utility variables
	bit_t srcRegNum = make_bits(4), dstRegNum = make_bits(4); // Used eg. in "NOT $rA $rB"
	for (int i = 0; i < 4; i++) {
		constant(&srcRegNum[i], 0);
		constant(&dstRegNum[i], 0);
	}
	bit_t mustIncrementProgramCounterByTwo = make_bits(1);
	constant(mustIncrementProgramCounterByTwo, 0);

	for (uint8_t i = 0; i < 8; i++)
		getN_thOperandBit(&operand[i], i, address, memory);
#if DEBUG
	printf("Operand: ");
	printBE(operand, 8);
	printf("\n");
#endif

	bit_t isNOP = make_bits(1);
	detectNOP(isNOP, operand);
#if DEBUG
	printf("Is NOP? %d.\n", decrypt(isNOP));
#endif
	inplace_or(mustIncrementProgramCounterByTwo, isNOP);

	bit_t isNOT = make_bits(1);
	detectNOT(isNOT, operand);
#if DEBUG
	printf("Is NOT? %d.\n", decrypt(isNOT));
#endif
	// Syntax: NOT $rA $rB
	// $rA <- ~$rB
	{
		bit_t A = make_bits(4);
		for (int i = 0; i < 4; i++) {
			getN_thOperandBit(&A[i], 8 + i, address, memory);
			mux(&dstRegNum[i], isNOT, &A[i], &dstRegNum[i]);
		}
		bit_t B = make_bits(4);
		for (uint8_t i = 0; i < 4; i++) {
			getN_thOperandBit(&B[i], 8 + 4 + i, address, memory);
			mux(&srcRegNum[i], isNOT, &B[i], &srcRegNum[i]);
		}

		inplace_or(mustIncrementProgramCounterByTwo, isNOT);
	}

	printf("Src reg: ");
	printBE(srcRegNum, 4);
	putchar('\n');
	printf("Dst reg: ");
	printBE(dstRegNum, 4);
	putchar('\n');

	for (int i = 0; i < NUMREGISTERS * REGISTERSIZE; i++) {
		copy(&newState.registers[i], &cpuState.registers[i]);
	}

	printf("Src reg contents: ");
	for (uint8_t i = 0; i < REGISTERSIZE; i++) {
		// Fetching srcBit
		bit_t srcBit = make_bits(1);
		// The code below is only valid for 4-bit register addresses!
#if NUMREGISTERS != (1 << 4)
#error Unsupported!
#endif
		getN_thBit(srcBit, i, REGISTERSIZE, srcRegNum, 4, cpuState.registers, 0);

		// Computing bit value
		bit_t dstBit = make_bits(1);
		bit_t NOTedBit = make_bits(1);
		_not(NOTedBit, srcBit);
		// Note the implicit copy from src here.
		mux(dstBit, isNOT, NOTedBit, srcBit);

		// Writing dstBit
		bit_t mask = make_bits(1);
		// Todo: mask = isNOT | ...
		copy(mask, isNOT);
		putN_thBit(dstBit, i, REGISTERSIZE, dstRegNum, 4, newState.registers, 0, mask);
	}

	putchar('\n');

	advancePCByTwo(newState.programCounter, cpuState.programCounter, mustIncrementProgramCounterByTwo);

	free_bits(mustIncrementProgramCounterByTwo);
	free_bits(isNOP);
	free_bits(isNOT);

	// We don't want to leak memory on every loop, do we?
	free_bits_array(cpuState.programCounter, BITNESS);
	newState.memory = cpuState.memory;

	return newState;
}