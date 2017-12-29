#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ElfParser.h"
#include "CPU.h"

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return 1;
	}
	char *code;
	uint8_t ret = extractCode(argv[1], &code);
	if (ret != 0) return ret;
	CPUState_t cpuState;
	cpuState.programCounter = malloc(BITNESS * sizeof(bit_t)); initialize(cpuState.programCounter, BITNESS);
	// Number of addressable bytes, times the number of bits in a byte.
	cpuState.memory = malloc((1 << BITNESS) * 8 * sizeof(bit_t)); initialize(cpuState.memory, (1 << BITNESS) * 8);
	for (int i = 0; i < BITNESS; i++)
		constant(cpuState.programCounter[i], 0);
	// NOP = 00001111xxxxxxxx
	// Todo: copy program code into memory.
	free(code); // <-- Todo: then do this
	bit_t instr[16]; initialize(instr, 16);
	constant16(instr, 0b0000111100000000);
	/* We shouldn't do memcpy since it leaks the overwritten pointers. Remember that the memory is an array
	 * of bit_t, and bit_t is a char*, so the memory is effectively an array of pointers. Overwriting them
	 * "raw" means leaking these pointers (in a rather unusual fashion, by the way).
	 */
	for (int i = 0; i < 16; i++)
		cpuState.memory[i] = instr[i];
	// memcpy(cpuState.memory, instr, 16 * sizeof(bit_t));

	for (int i = 0; i < 4; i++) {
		cpuState = doStep(cpuState);

		for (int i = BITNESS; i --> 0;)
			printf("%d", *cpuState.programCounter[i]);
		puts("");
	}

	return 0;
}