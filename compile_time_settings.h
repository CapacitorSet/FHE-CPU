//
// Created by yuri on 8/17/18.
//

#ifndef FHE_CPU_COMPILE_TIME_SETTINGS_H
#define FHE_CPU_COMPILE_TIME_SETTINGS_H

#define IS_PLAINTEXT (1)
#define DEBUG (1)

#ifndef IS_PLAINTEXT
#error IS_PLAINTEXT must be defined (either 0 or 1).
#endif
#ifndef DEBUG
#error DEBUG must be defined (either 0 or 1).
#endif

#if DEBUG && IS_PLAINTEXT
#define bootsSymDecrypt(ptr, _) (*(ptr))
#endif
#endif //FHE_CPU_COMPILE_TIME_SETTINGS_H
