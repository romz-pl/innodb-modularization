#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_skip_corrupt_index.h>

/* Get the next non-corrupt index */
#define dict_table_next_uncorrupted_index(index) \
  do {                                           \
    index = index->next();                       \
    dict_table_skip_corrupt_index(index);        \
  } while (0)
