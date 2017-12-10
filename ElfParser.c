#include <stdio.h>
#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>

// Convert a 64-bit value to the host endianness, depending on the file's endianness
#define _64toh(x) (isBigEndian ? be64toh(x) : le64toh(x))
#define _32toh(x) (isBigEndian ? be32toh(x) : le32toh(x))
#define _16toh(x) (isBigEndian ? be16toh(x) : le16toh(x))

uint8_t fskip(FILE *f, uint8_t bytes) {
    fseek(f, bytes, SEEK_CUR);
    return bytes;
}

uint8_t read8or4(FILE *f, uint64_t *target, char is64bit, char isBigEndian) {
    if (is64bit) {
        uint64_t tmp;
        fread(&tmp, 1, 8, f);
        *target = _64toh(tmp);
        return 8;
    }
    uint32_t tmp;
    fread(&tmp, 1, 4, f);
    *target = _32toh(tmp);
    return 4;
}

uint8_t extractCode(const char *filename, char** dst) {
    FILE *f = fopen(filename, "r");
    char* header = (char*) malloc(4);
    fread(header, sizeof(char), 4, f);
    if (memcmp(header, "\x7f" "ELF", 4) != 0) {
        fprintf(stderr, "Not an ELF file!\n");
        return 2;
    }
    char is64bit = fgetc(f) - 1;
    char isBigEndian = fgetc(f) - 1;

    // Get start of program header table (e_phoff)
    uint64_t e_phoff;
    fseek(f, is64bit ? 0x28 : 0x20, SEEK_SET);
    if (is64bit) {
        fread(&e_phoff, sizeof(char), 8, f);
        e_phoff = _64toh(e_phoff);
    } else {
        uint32_t tmp;
        fread(&tmp, sizeof(char), 8, f);
        e_phoff = _32toh(tmp);
    }

    // Get program header
    fseek(f, e_phoff, 0);
    uint32_t sh_type;
    uint64_t sh_offset;
    uint64_t sh_size;
    do {
        uint8_t bytes_read = 0;
        // Skip sh_name
        bytes_read += fskip(f, 4);
        fread(&sh_type, sizeof(char), 4, f); bytes_read += 4; // Read sh_type
        sh_type = _32toh(sh_type);
        if (feof(f)) {
            fprintf(stderr, "Couldn't find executable section (SHT_PROGBITS)!");
            return 3;
        }
        // Skip sh_flags
        bytes_read += fskip(f, is64bit ? 8 : 4);
        // Skip sh_addr
        bytes_read += fskip(f, is64bit ? 8 : 4);
        bytes_read += read8or4(f, &sh_offset, is64bit, isBigEndian);
        bytes_read += read8or4(f, &sh_size, is64bit, isBigEndian);
        // Go to next file (i.e. current file + 28/48 bytes)
        fseek(f, (-bytes_read) + (is64bit ? 0x40 : 0x28), SEEK_CUR);
    } while (sh_type != SHT_PROGBITS);
    fseek(f, sh_offset, SEEK_SET);
    (*dst) = (char*) malloc(sh_size);
    fread(*dst, 1, sh_size, f);
    fclose(f);
    return 0;
}