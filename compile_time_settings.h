//
// Created by yuri on 8/17/18.
//

#ifndef FHE_CPU_COMPILE_TIME_SETTINGS_H
#define FHE_CPU_COMPILE_TIME_SETTINGS_H

#define IS_PLAINTEXT (1)
#define DEBUG (1)
#define TRIVIAL_getNth_bit (0) // Helps fail faster on invalid getN_thbit reads.
#define TRIVIAL_putNth_bit (0) // Helps fail faster on invalid putN_thbit writes.

#define BITNESS (4) // 16, 32, 64 bits?
// You shouldn't change the line below
#define MEMSIZE ((1 << BITNESS) * 8) // Number of addressable bytes, times the number of bits in a byte.
// You shouldn't change the line below
#define NUMREGISTERS (1 << 4) // There are 4 register bits, so 1<<4 possible registers
#define REGISTERSIZE 8 // The size of an individual register

// Sanity checks
#ifndef IS_PLAINTEXT
#error IS_PLAINTEXT must be defined (either 0 or 1).
#endif
#ifndef DEBUG
#error DEBUG must be defined (either 0 or 1).
#endif
#ifndef BITNESS
#error BITNESS must be defined.
#endif
#ifndef TRIVIAL_getNth_bit
#error TRIVIAL_getNth_bit must be defined (either 0 or 1).
#endif
#ifndef TRIVIAL_putNth_bit
#error TRIVIAL_putNth_bit must be defined (either 0 or 1).
#endif
#if MEMSIZE > ((1 << BITNESS) * 8)
#warning The memory is larger than the addressable space.
#elif MEMSIZE < ((1 << BITNESS) * 8)
#error The memory is smaller than the addressable space.
#endif

#if TRIVIAL_getNth_bit && !DEBUG
#error TRIVIAL_getNth_bit requires DEBUG=1.
#endif
#if TRIVIAL_putNth_bit && !DEBUG
#error TRIVIAL_putNth_bit requires DEBUG=1.
#endif
#endif //FHE_CPU_COMPILE_TIME_SETTINGS_H
