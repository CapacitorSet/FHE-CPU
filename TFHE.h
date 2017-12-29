#ifndef FHE_CPU_TFHE_H
#define FHE_CPU_TFHE_H

#define STUB_ROOT_DATATYPE char // Ciphertexts are emulated as a pointer. A pointer to what data type?
typedef STUB_ROOT_DATATYPE* bit_t;

void initialize(bit_t* dst, int size);
bit_t make_bits(int N);
void free_bits(bit_t item);
void free_bits_array(bit_t *item, int size);

void constant(bit_t dst, int src);

void not(bit_t dst, bit_t a);
void and(bit_t dst, bit_t a, bit_t b);
void nand(bit_t dst, bit_t a, bit_t b);
void or(bit_t dst, bit_t a, bit_t b);
void inplace_or(bit_t dst, bit_t b);
void nor(bit_t dst, bit_t a, bit_t b);
void xor(bit_t dst, bit_t a, bit_t b);
void nxor(bit_t dst, bit_t a, bit_t b);

void mux(bit_t dst, bit_t cond, bit_t a, bit_t b);
void copy(bit_t dst, bit_t src);

#endif //FHE_CPU_TFHE_H
