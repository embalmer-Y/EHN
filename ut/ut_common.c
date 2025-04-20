#include "ut_common.h"

int ut_common_compile_ret(int src, int expect) {
    if (src == expect) {
        return 0;
    }

    return -1;
}

int ut_common_compile_uint8(uint8_t src, uint8_t expect) { 
    if (src == expect) {
        return 0; 
    }

    return -1;
}

int ut_common_compile_uint16(uint16_t src, uint16_t expect) {
    if (src == expect) {
        return 0;
    }
    
    return -1;
}

int ut_common_compile_uint32(uint32_t src, uint32_t expect) {
    if (src == expect) {
        return 0;
    }
    
    return -1;
}
