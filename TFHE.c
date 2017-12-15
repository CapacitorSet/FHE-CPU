#include <string.h>
#include "TFHE.h"

bit_t and(bit_t a, bit_t b) {
	return a && b;
}

bit_t nor(bit_t a, bit_t b) {
	return !(a || b);
}

bit_t nxor(bit_t a, bit_t b) {
	return !(a ^ b);
}

bit_t mux(bit_t cond, bit_t a, bit_t b) {
	return cond ? a : b;
}

void copy(bit_t *dst, bit_t src) {
	memcpy(dst, &src, sizeof(bit_t));
}