#pragma once

#include <innodb/univ/univ.h>

/** Data bytes of an infimum record */
static const byte infimum_data[] = {
    0x69, 0x6e, 0x66, 0x69, 0x6d, 0x75, 0x6d, 0x00 /* "infimum\0" */
};
