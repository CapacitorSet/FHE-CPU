#include <stdio.h>
#include "ElfParser.h"

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return 1;
	}
	char *buf;
	uint8_t ret = extractCode(argv[1], &buf);
	if (ret != 0) return ret;
	return 0;
}