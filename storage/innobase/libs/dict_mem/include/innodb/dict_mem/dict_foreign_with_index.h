#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_t.h>

/** A function object to find a foreign key with the given index as the
referenced index. Return the foreign key with matching criteria or NULL */
struct dict_foreign_with_index {
  dict_foreign_with_index(const dict_index_t *index) : m_index(index) {}

  bool operator()(const dict_foreign_t *foreign) const {
    return (foreign->referenced_index == m_index);
  }

  const dict_index_t *m_index;
};
