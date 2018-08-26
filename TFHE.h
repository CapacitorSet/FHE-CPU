#ifndef FHE_CPU_TFHE_H
#define FHE_CPU_TFHE_H

#include "compile_time_settings.h"

#if !IS_PLAINTEXT
#include <tfhe/tfhe.h>
const TFheGateBootstrappingCloudKeySet* bk;
const TFheGateBootstrappingParameterSet* params;
#if DEBUG
TFheGateBootstrappingSecretKeySet* secret_key;
#endif
typedef LweSample* bit_t;
typedef LweSample* bits_t;

void tfhe_setup();


bit_t make_bits(int N);
void free_bits(bit_t item);
void free_bits_array(bits_t item, int size);

void constant(bit_t dst, int src);

void _not(bit_t dst, bit_t a);
void _and(bit_t dst, bit_t a, bit_t b);
void _andyn(bit_t dst, bit_t a, bit_t b);
void _andny(bit_t dst, bit_t a, bit_t b);
void _nand(bit_t dst, bit_t a, bit_t b);
void _or(bit_t dst, bit_t a, bit_t b);
void inplace_or(bit_t dst, bit_t const b);
void _nor(bit_t dst, bit_t a, bit_t b);
void _xor(bit_t dst, bit_t a, bit_t b);
void _nxor(bit_t dst, bit_t a, bit_t b);

void mux(bit_t dst, bit_t cond, bit_t a, bit_t b);
void copy(bit_t dst, bit_t src);
#else
#define secret_key NULL
/*
const TFheGateBootstrappingCloudKeySet* bk;
const TFheGateBootstrappingParameterSet* params;
#if DEBUG
TFheGateBootstrappingSecretKeySet* secret_key;
#endif
*/
typedef char* bit_t;
typedef char* bits_t;

void tfhe_setup();

bit_t make_bits(int N);
void free_bits(bit_t item);
void free_bits_array(bits_t item, int size);

void constant(bit_t dst, int src);

void _not(bit_t dst, bit_t a);
void _and(bit_t dst, bit_t a, bit_t b);
void _andyn(bit_t dst, bit_t a, bit_t b);
void _andny(bit_t dst, bit_t a, bit_t b);
void _nand(bit_t dst, bit_t a, bit_t b);
void _or(bit_t dst, bit_t a, bit_t b);
void inplace_or(bit_t dst, bit_t const b);
void _nor(bit_t dst, bit_t a, bit_t b);
void _xor(bit_t dst, bit_t a, bit_t b);

void mux(bit_t dst, bit_t cond, bit_t a, bit_t b);
void copy(bit_t dst, bit_t src);
#endif

#endif //FHE_CPU_TFHE_H
