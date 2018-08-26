cmake_minimum_required(VERSION 3.9)
project(FHE_CPU C)

set(CMAKE_C_STANDARD 11)

add_executable(FHE_CPU main.c ElfParser.c ElfParser.h CPU.c CPU.h TFHE.c TFHE.h compile_time_settings.h debug.c debug.h utils.c utils.h)
target_link_libraries(FHE_CPU tfhe-spqlios-fma)