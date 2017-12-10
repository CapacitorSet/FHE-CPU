#ifndef FHE_CPU_ELFPARSER_H
#define FHE_CPU_ELFPARSER_H

#include <stdint.h>

uint8_t extractCode(const char *filename, char** dst);

#endif //FHE_CPU_ELFPARSER_H
