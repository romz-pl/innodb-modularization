#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/* Skip corrupted index */
#define dict_table_skip_corrupt_index(index) \
  while (index && index->is_corrupted()) {   \
    index = index->next();                   \
  }
