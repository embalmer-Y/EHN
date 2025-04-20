#ifndef UT_COMMON_H
#define UT_COMMON_H

#include <stdint.h>

int ut_common_compile_ret(int src, int expect);
int ut_common_compile_uint8(uint8_t src, uint8_t expect);
int ut_common_compile_uint16(uint16_t src, uint16_t expect);
int ut_common_compile_uint32(uint32_t src, uint32_t expect);
// int ut_common_compile_uint64(uint64_t src, uint64_t expect);

#endif // UT_COMMON_H