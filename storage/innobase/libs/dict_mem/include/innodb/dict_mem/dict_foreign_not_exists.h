#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_set.h>

/** Function object to check if a foreign key object is there
in the given foreign key set or not.  It returns true if the
foreign key is not found, false otherwise */
struct dict_foreign_not_exists {
  dict_foreign_not_exists(const dict_foreign_set &obj_) : m_foreigns(obj_) {}

  /* Return true if the given foreign key is not found */
  bool operator()(dict_foreign_t *const &foreign) const {
    return (m_foreigns.find(foreign) == m_foreigns.end());
  }

 private:
  const dict_foreign_set &m_foreigns;
};
