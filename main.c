#include <stdio.h>
#include "compile_time_settings.h"
#include "debug.h"
#include "utils.h"
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
	cpuState.registers = make_bits(NUMREGISTERS * REGISTERSIZE);
	// Todo: is this needed? Maybe the allocator could just clear the bits
	clear(cpuState.programCounter, BITNESS);
	clear(cpuState.memory, MEMSIZE);
	clear(cpuState.registers, NUMREGISTERS * REGISTERSIZE);

	// Todo: copy program code into memory, then free(code).
	constant16(&cpuState.memory[0], 0b0000111100000000); // NOP = 00001111xxxxxxxx
	constant16(&cpuState.memory[16], 0b0010110010100000); // NEG 1010, 0000
	for (int i = 0; i < REGISTERSIZE; i++)
		constant(&cpuState.registers[0b0000 * REGISTERSIZE + i], i == 2);

	printf("\nRegisters: \n");
	printLongBE(cpuState.registers, NUMREGISTERS * REGISTERSIZE);

	for (int i = 0; i < 4; i++) {
		cpuState = doStep(cpuState);

#if DEBUG
		printf("Finish step. New PC: ");
		printLE(cpuState.programCounter, BITNESS);
		/*
		printf("\nMemory: \n");
		printLongBE(cpuState.memory, MEMSIZE);
		*/
		printf("\nRegisters: \n");
		printLongBE(cpuState.registers, NUMREGISTERS * REGISTERSIZE);
		puts("-----------------");
#endif
	}

	free_bits_array(cpuState.programCounter, BITNESS);
	free_bits_array(cpuState.memory, MEMSIZE);
	return 0;
}