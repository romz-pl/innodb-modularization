#pragma once

#include <innodb/univ/univ.h>

/** Table name wrapper for pretty-printing */
struct table_name_t {
  /** The name in internal representation */
  char *m_name;
};
