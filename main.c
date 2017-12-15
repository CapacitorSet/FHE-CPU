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
	cpuState.programCounter = calloc(BITNESS, sizeof(bit_t));
	cpuState.memory = calloc(1 << BITNESS, 8 * sizeof(typeof(cpuState.memory)));
	// NOP = 00001111xxxxxxxx
	bit_t instr[16] = {
			0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0
	};
	memcpy(cpuState.memory, instr, 16 * sizeof(bit_t));

	cpuState = doStep(cpuState);

	return 0;
}