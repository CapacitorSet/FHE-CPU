/* Implementation-independent utilities.
 */
#include "utils.h"

void constant16(bit_t dst, const uint16_t src) {
	for (int i = 0; i < 16; i++)
		constant(&dst[i], (src >> (15 - i)) & 1);
}

void clear(bit_t dst, size_t N) {
	for (size_t i = 0; i < N; i++)
		constant(&dst[i], 0);
}

// I know, it's awkward, but it's useful in optimizing register comparisons.
void optimizedCompareN(bit_t dst, bit_t src, uint8_t N, uint8_t target) {
	assert(N % 2 == 0);
	switch (target & 0b11) {
		case 0b00:
			optimizedCompareMacro_00(dst, &src[0], &src[1]);
			break;
		case 0b01:
			optimizedCompareMacro_01(dst, &src[0], &src[1]);
			break;
		case 0b10:
			optimizedCompareMacro_10(dst, &src[0], &src[1]);
			break;
		case 0b11:
			optimizedCompareMacro_11(dst, &src[0], &src[1]);
			break;
	}
	for (int i = 2; i < N; i += 2) {
		bit_t tmp = make_bits(1);
		switch (target & 0b11) {
			case 0b00:
				optimizedCompareMacro_00(tmp, &src[i], &src[i + 1]);
				break;
			case 0b01:
				optimizedCompareMacro_01(tmp, &src[i], &src[i + 1]);
				break;
			case 0b10:
				optimizedCompareMacro_10(tmp, &src[i], &src[i + 1]);
				break;
			case 0b11:
				optimizedCompareMacro_11(tmp, &src[i], &src[i + 1]);
				break;
		}
		_and(dst, tmp, dst);
	}
}