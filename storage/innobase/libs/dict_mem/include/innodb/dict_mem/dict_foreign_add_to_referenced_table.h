#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/dict_mem/dict_foreign_set.h>
#include <innodb/dict_mem/dict_table_t.h>

/** A function object to add the foreign key constraint to the referenced set
of the referenced table, if it exists in the dictionary cache. */
struct dict_foreign_add_to_referenced_table {
  void operator()(dict_foreign_t *foreign) const {
    if (dict_table_t *table = foreign->referenced_table) {
      std::pair<dict_foreign_set::iterator, bool> ret =
          table->referenced_set.insert(foreign);
      ut_a(ret.second);
    }
  }
};
