#pragma once

#include <innodb/univ/univ.h>

/** Extra bytes and data bytes of a supremum record */
static const byte supremum_extra_data[] = {
    /* 0x0?, */ /* info_bits=0, n_owned=1..8 */
    0x00,
    0x0b, /* heap_no=1, status=3 */
    0x00,
    0x00, /* next=0 */
    0x73,
    0x75,
    0x70,
    0x72,
    0x65,
    0x6d,
    0x75,
    0x6d /* "supremum" */
};
