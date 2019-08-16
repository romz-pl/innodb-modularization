#pragma once

#include <innodb/univ/univ.h>

UNIV_COLD int cmp_decimal(const byte *a, unsigned int a_length,
                                 const byte *b, unsigned int b_length,
                                 bool is_asc);
