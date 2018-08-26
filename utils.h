#ifndef FHE_CPU_UTILS_H
#define FHE_CPU_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "TFHE.h"

// Store src into dst.
void constant16(bit_t dst, uint16_t src);
// Clear N bits in dst.
void clear(bit_t dst, size_t N);

// Optimal functions for comparing unknown bits a1, a2 to known bits b1, b2 (embedded in the name of the function)
#define optimizedCompareMacro_00 _nor
#define optimizedCompareMacro_01 _andny
#define optimizedCompareMacro_10 _andyn
#define optimizedCompareMacro_11 _and

// Compare `target` with `src`, which is N bits long. Store 1 in `dst` if they are equal, 0 otherwise.
void optimizedCompareN(bit_t dst, bit_t src, uint8_t N, uint8_t target);

// Creates an optimal decoder by simplification at compile time
#define optDetector(name, a0, a1, a2, a3, a4, a5, a6, a7) void detect ## name(bit_t ret, const bit_t operand) { \
	bit_t compare01 = make_bits(1); optimizedCompareMacro_ ## a0 ## a1(compare01, &operand[0], &operand[1]); \
	bit_t compare23 = make_bits(1); optimizedCompareMacro_ ## a2 ## a3(compare23, &operand[2], &operand[3]); \
	bit_t compare45 = make_bits(1); optimizedCompareMacro_ ## a4 ## a5(compare45, &operand[4], &operand[5]); \
	bit_t compare67 = make_bits(1); optimizedCompareMacro_ ## a6 ## a7(compare67, &operand[6], &operand[7]); \
	bit_t compare0123 = make_bits(1); _and(compare0123, compare01, compare23); \
	bit_t compare4567 = make_bits(1); _and(compare4567, compare45, compare67); \
	_and(ret, compare0123, compare4567);\
	free(compare01); free(compare23); free(compare45); free(compare67); free(compare0123); free(compare4567); \
}

#endif //FHE_CPU_UTILS_H
