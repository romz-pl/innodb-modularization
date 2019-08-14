#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_set.h>
#include <innodb/dict_mem/dict_foreign_free.h>

#include <algorithm>

/** The destructor will free all the foreign key constraints in the set
by calling dict_foreign_free() on each of the foreign key constraints.
This is used to free the allocated memory when a local set goes out
of scope. */
struct dict_foreign_set_free {
  dict_foreign_set_free(const dict_foreign_set &foreign_set)
      : m_foreign_set(foreign_set) {}

  ~dict_foreign_set_free() {
    std::for_each(m_foreign_set.begin(), m_foreign_set.end(),
                  dict_foreign_free);
  }

  const dict_foreign_set &m_foreign_set;
};
