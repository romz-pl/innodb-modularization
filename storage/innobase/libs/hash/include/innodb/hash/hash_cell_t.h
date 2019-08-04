#pragma once

#include <innodb/univ/univ.h>

struct hash_cell_t {
  void *node; /*!< hash chain node, NULL if none */
};
