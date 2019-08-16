#pragma once

#include <innodb/univ/univ.h>

int cmp_data(ulint mtype, ulint prtype, bool is_asc, const byte *data1,
                    ulint len1, const byte *data2, ulint len2);
