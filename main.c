#include <stdio.h>
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
	cpuState.programCounter = 0;

	doStep(cpuState);

	return 0;
}