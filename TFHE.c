#include "compile_time_settings.h"
#include <stdio.h>
#include "TFHE.h"

#if !IS_PLAINTEXT

// Todo: is this really needed, or can we just or(dst, dst, b)?
void inplace_or(bit_t dst, const bit_t b) {
	bit_t tmp = make_bits(1);
	_or(tmp, dst, b);
	copy(dst, tmp);
	free_bits(tmp);
}

void tfhe_setup(void) {
	FILE *cloud_key = fopen("cloud.key","rb");
	bk = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
	fclose(cloud_key);
#if DEBUG
	FILE *secretKey = fopen("secret.key","rb");
	secret_key = new_tfheGateBootstrappingSecretKeySet_fromFile(secretKey);
	fclose(secretKey);
#endif
	params = bk->params;
}

void create_keys() {
	printf("Generating keyset...\n");
	const int minimum_lambda = 110;
	uint32_t seed[] = {314, 1592, 657};
	TFheGateBootstrappingParameterSet *params =
			new_default_gate_bootstrapping_parameters(minimum_lambda);
	tfhe_random_generator_setSeed(seed, sizeof(seed)/sizeof(seed[0]));
	TFheGateBootstrappingSecretKeySet *key =
			new_random_gate_bootstrapping_secret_keyset(params);

	printf("Exporting secret key...\n");
	FILE *secret_key = fopen("secret.key", "wb");
	export_tfheGateBootstrappingSecretKeySet_toFile(secret_key, key);
	fclose(secret_key);

	printf("Exporting public key...\n");
	FILE *cloud_key = fopen("cloud.key", "wb");
	export_tfheGateBootstrappingCloudKeySet_toFile(cloud_key, &key->cloud);
	fclose(cloud_key);
}

void constant(bit_t dst, int src) {
	bootsCONSTANT(dst, src, bk);
}

void _and(bit_t dst, bit_t a, bit_t b) {
	bootsAND(dst, a, b, bk);
}

void _andyn(bit_t dst, bit_t a, bit_t b) {
	bootsANDYN(dst, a, b, bk);
}

void _nor(bit_t dst, bit_t a, bit_t b) {
	bootsNOR(dst, a, b, bk);
}

void _xor(bit_t dst, bit_t a, bit_t b) {
	bootsXOR(dst, a, b, bk);
}

void _or(bit_t dst, bit_t a, bit_t b) {
	bootsOR(dst, a, b, bk);
}

bit_t make_bits(int N) {
	return new_gate_bootstrapping_ciphertext_array(N, params);
}

void free_bits(bit_t item) {
	free_LweSample(item);
}

void free_bits_array(bits_t item, int size) {
	free_LweSample_array(size, item);
}

void mux(bit_t dst, bit_t cond, bit_t a, bit_t b) {
	bootsMUX(dst, cond, a, b, bk);
}

void copy(bit_t dst, bit_t src) {
	bootsCOPY(dst, src, bk);
}
#else
#include <malloc.h>
// Todo: is this really needed, or can we just or(dst, dst, b)?
void inplace_or(bit_t dst, const bit_t b) {
	bit_t tmp = make_bits(1);
	_or(tmp, dst, b);
	copy(dst, tmp);
	free_bits(tmp);
}

void tfhe_setup(void) {
}

void create_keys(void) {
}

void constant(bit_t dst, int src) {
	*dst = src;
}

void _not(bit_t dst, bit_t src) {
	*dst = !*src;
}

void _and(bit_t dst, bit_t a, bit_t b) {
	*dst = *a && *b;
}

void _andyn(bit_t dst, bit_t a, bit_t b) {
	*dst = *a && !*b;
}

void _andny(bit_t dst, bit_t a, bit_t b) {
	*dst = !*a && *b;
}

void _nand(bit_t dst, bit_t a, bit_t b) {
	*dst = !(*a && *b);
}

void _or(bit_t dst, bit_t a, bit_t b) {
	*dst = *a || *b;
}

void _nor(bit_t dst, bit_t a, bit_t b) {
	*dst = !(*a || *b);
}

void _xor(bit_t dst, bit_t a, bit_t b) {
	*dst = *a ^ *b;
}

bit_t make_bits(int N) {
	return malloc(N);
}

void free_bits(bit_t item) {
	free(item);
}

void free_bits_array(bits_t item, int size) {
	free(item);
}

void mux(bit_t dst, bit_t cond, bit_t a, bit_t b) {
	*dst = *cond ? *a : *b;
}

void copy(bit_t dst, bit_t src) {
	*dst = *src;
}
#endif