#ifndef FHE_CPU_DEBUG_H
#define FHE_CPU_DEBUG_H

#include "compile_time_settings.h"
#include "TFHE.h"
#include <stddef.h>
#include <stdio.h>

#if DEBUG
#if IS_PLAINTEXT
#define decrypt(ptr) (*(ptr))
#else
#define decrypt(input) bootsSymDecrypt(input, secret_key)
#endif

void printBE(bit_t input, size_t size);
void printLE(bit_t input, size_t size);
void printLongBE(bit_t input, size_t size);
#endif

#endif //FHE_CPU_DEBUG_H
