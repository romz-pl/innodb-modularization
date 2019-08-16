#pragma once

#include <innodb/univ/univ.h>


int cmp_whole_field(ulint mtype, ulint prtype, bool is_asc,
                           const byte *a, unsigned int a_length, const byte *b,
                           unsigned int b_length);
