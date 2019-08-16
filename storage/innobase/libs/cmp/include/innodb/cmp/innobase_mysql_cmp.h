#pragma once

#include <innodb/univ/univ.h>

int innobase_mysql_cmp(ulint prtype, const byte *a, size_t a_length,
                       const byte *b, size_t b_length);
