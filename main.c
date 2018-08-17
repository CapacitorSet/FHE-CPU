#include "compile_time_settings.h"
#include <stdio.h>
#include "debug.h"
#if !IS_PLAINTEXT
#include <tfhe/tfhe_io.h>
#include <tfhe/tfhe.h>
#endif
#include "ElfParser.h"
#include "CPU.h"

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return 1;
	}
	// create_keys();
	tfhe_setup();

	char *code;
	uint8_t ret = extractCode(argv[1], &code);
	if (ret != 0) return ret;

	CPUState_t cpuState;
	cpuState.programCounter = make_bits(BITNESS);
	cpuState.memory = make_bits(MEMSIZE);
	// Todo: is this needed? Maybe the allocator could just clear the bits
	for (int i = 0; i < BITNESS; i++)
		constant(&cpuState.programCounter[i], 0);
	for (int i = 0; i < MEMSIZE; i++)
		constant(&cpuState.memory[i], 0);

	bits_t instr = make_bits(16);
	// Todo: copy program code into memory, then free(code).
	constant16(instr, 0b0000111100000000); // NOP = 00001111xxxxxxxx
	/* We shouldn't do memcpy since it leaks the overwritten pointers. Remember that the memory is an array
	 * of bit_t, and bit_t is a char*, so the memory is effectively an array of pointers. Overwriting them
	 * "raw" means leaking these pointers (in a rather unusual fashion, by the way).
	 */
	for (int i = 0; i < 16; i++)
		copy(&cpuState.memory[i], &instr[i]);
	constant16(instr, 0b0010110010100101); // NEG 1010, 0101
	for (int i = 0; i < 16; i++)
		copy(&cpuState.memory[i + 16], &instr[i]);
	free_bits_array(instr, 16);

	for (int i = 0; i < 4; i++) {
		cpuState = doStep(cpuState);

#if DEBUG
		printf("Finish step. New PC: ");
		printLE(cpuState.programCounter, BITNESS);
		printf("\nMemory: \n");
		printLongBE(cpuState.memory, MEMSIZE);
		puts("-----------------");
#endif
	}

	free_bits_array(cpuState.programCounter, BITNESS);
	free_bits_array(cpuState.memory, MEMSIZE);
	return 0;
}