#ifndef FHE_CPU_TFHE_H
#define FHE_CPU_TFHE_H

typedef char bit_t;

bit_t not(bit_t a);
bit_t and(bit_t a, bit_t b);
bit_t nand(bit_t a, bit_t b);
bit_t or(bit_t a, bit_t b);;
bit_t nor(bit_t a, bit_t b);
bit_t xor(bit_t a, bit_t b);
bit_t nxor(bit_t a, bit_t b);

bit_t mux(bit_t cond, bit_t a, bit_t b);
void copy(bit_t *dst, bit_t src);

#endif //FHE_CPU_TFHE_H
