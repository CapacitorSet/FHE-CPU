//
// Created by yuri on 8/17/18.
//

#ifndef FHE_CPU_COMPILE_TIME_SETTINGS_H
#define FHE_CPU_COMPILE_TIME_SETTINGS_H

#define IS_PLAINTEXT (1)
#define DEBUG (1)
#define TRIVIAL_getNth_bit (0)

#define BITNESS (4) // 16, 32, 64 bits?
// You shouldn't change the line below
#define MEMSIZE ((1 << BITNESS) * 8) // Number of addressable bytes, times the number of bits in a byte.

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

#if TRIVIAL_getNth_bit && !DEBUG
#error TRIVIAL_getNth_bit requires DEBUG=1.
#endif
#endif //FHE_CPU_COMPILE_TIME_SETTINGS_H
