#include "debug.h"

void printBE(bit_t input, size_t size) {
	for (size_t i = 0; i < size; i++)
		printf("%d", decrypt(&input[i]));
}

void printLE(bit_t input, size_t size) {
	for (size_t i = size; i --> 0;)
		printf("%d", decrypt(&input[i]));
}

void printLongBE(bit_t input, size_t size) {
	for (size_t i = 0; i < size; i++) {
		if (i % 16 == 0)
			printf("%#04x ", i >> 4);
		printf("%d", decrypt(&input[i]));
		if (i % 16 == 15)
			putchar('\n');
	}
}