#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_t.h>

/* A function object to check if the foreign constraint is between different
tables.  Returns true if foreign key constraint is between different tables,
false otherwise. */
struct dict_foreign_different_tables {
  bool operator()(const dict_foreign_t *foreign) const {
    return (foreign->foreign_table != foreign->referenced_table);
  }
};
