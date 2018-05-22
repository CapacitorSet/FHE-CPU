#include <stdio.h>
#include <tfhe/tfhe_io.h>
#include <tfhe/tfhe.h>
#include "ElfParser.h"
#include "CPU.h"

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

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return 1;
	}
	// create_keys();
	tfhe_setup();

	char *code;
	uint8_t ret = extractCode(argv[1], &code);
	if (ret != 0) return ret;

	CPUState_t cpuState;
	cpuState.programCounter = make_bits(BITNESS);
	cpuState.memory = make_bits(MEMSIZE);
	// Todo: is this needed? Maybe the allocator could just clear the bits
	for (int i = 0; i < BITNESS; i++)
		constant(&cpuState.programCounter[i], 0);
	for (int i = 0; i < MEMSIZE; i++)
		constant(&cpuState.memory[i], 0);

	bits_t instr = make_bits(16);
	// Todo: copy program code into memory, then free(code).
	constant16(instr, 0b0000111100000000); // NOP = 00001111xxxxxxxx
	/* We shouldn't do memcpy since it leaks the overwritten pointers. Remember that the memory is an array
	 * of bit_t, and bit_t is a char*, so the memory is effectively an array of pointers. Overwriting them
	 * "raw" means leaking these pointers (in a rather unusual fashion, by the way).
	 */
	for (int i = 0; i < 16; i++)
		copy(&cpuState.memory[i], &instr[i]);
	constant16(instr, 0b0010110010100101); // NEG 1010, 0101
	for (int i = 0; i < 16; i++)
		copy(&cpuState.memory[i + 16], &instr[i]);
	free_bits_array(instr, 16);

	for (int i = 0; i < 4; i++) {
		cpuState = doStep(cpuState);

#if DEBUG
		printf("Finish step. New PC: ");
		for (int j = BITNESS; j --> 0;)
			printf("%d", bootsSymDecrypt(&cpuState.programCounter[j], secret_key));
		printf("\nMemory:");
		for (int j = 0; j < MEMSIZE; j++) {
			if (j % 16 == 0)
				putchar(' ');
			printf("%d", bootsSymDecrypt(&cpuState.memory[j], secret_key));
		}
		puts("");
		puts("-----------------");
#endif
	}

	free_bits_array(cpuState.programCounter, BITNESS);
	free_bits_array(cpuState.memory, MEMSIZE);
	return 0;
}