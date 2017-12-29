#include <stdlib.h>
#include <stdio.h>
#include "TFHE.h"

bit_t make_bits(int N) {
	if (N != 1) {
		printf("Calling make_bits with %d bits? You're doing it wrong!", N);
		exit(-1);
	}
	return (bit_t) malloc(sizeof(bit_t));
}

void free_bits(bit_t item) {
	free(item);
}

void free_bits_array(bit_t *item, int size) {
	for (int i = 0; i < size; i++)
		free_bits(item[i]);
}

// Allows for compact initialization: initialize(NOP, 8), constant8(NOP, 0b00001111)
void initialize(bit_t* dst, int size) {
	for (int i = 0; i < size; i++)
		dst[i] = malloc(sizeof(bit_t));
}

void constant(bit_t dst, const int src) {
	*dst = src;
}

void not(bit_t dst, const bit_t a) {
	*dst = !*a;
}

void and(bit_t dst, const bit_t a, const bit_t b) {
	*dst = *a && *b;
}

void or(bit_t dst, const bit_t a, const bit_t b) {
	*dst = *a || *b;
}

// Todo: is this really needed, or can we just or(dst, dst, b)?
void inplace_or(bit_t dst, const bit_t b) {
	bit_t tmp = make_bits(1);
	or(tmp, dst, b);
	copy(dst, tmp);
}

void nor(bit_t dst, const bit_t a, const bit_t b) {
	*dst = !(*a || *b);
}

void xor(bit_t dst, const bit_t a, const bit_t b) {
	*dst = *a ^ *b;
}

void nxor(bit_t dst, const bit_t a, const bit_t b) {
	*dst = !(*a ^ *b);
}

void mux(bit_t dst, const bit_t cond, const bit_t a, const bit_t b) {
	*dst = *cond ? *a : *b;
}

void copy(bit_t dst, const bit_t src) {
	*dst = *src;
}