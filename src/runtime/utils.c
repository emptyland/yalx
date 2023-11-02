#include "runtime/utils.h"
#include "runtime/checking.h"

int yalx_count_leading_zeros_32(uint32_t x) {
    static const int zval[16] = {
            4, /* 0 */ 3, /* 1 */ 2, /* 2 */ 2, /* 3 */
            1, /* 4 */ 1, /* 5 */ 1, /* 6 */ 1, /* 7 */
            0, /* 8 */ 0, /* 9 */ 0, /* A */ 0, /* B */
            0, /* C */ 0, /* D */ 0, /* E */ 0  /* F */
    };

    int base = 0;
    if ((x & 0xFFFF0000) == 0) {base  = 16; x <<= 16;} else {base = 0;}
    if ((x & 0xFF000000) == 0) {base +=  8; x <<=  8;}
    if ((x & 0xF0000000) == 0) {base +=  4; x <<=  4;}
    return base + zval[x >> (32-4)];
}

size_t yalx_round_up_power_of_2(size_t input) {
    DCHECK(input != 0);
    if (yalx_u64_is_power_of_2(input)) {
        return input;
    }
    uint32_t lz = yalx_count_leading_zeros_64(input);
    DCHECK(lz < sizeof(input) * 8);
    DCHECK(lz > 0);

    return ((size_t)1) << (sizeof(input) * 8 - lz);
}