#include "TFHE.h"

bit_t mux(bit_t cond, bit_t a, bit_t b) {
	return cond ? a : b;
}
