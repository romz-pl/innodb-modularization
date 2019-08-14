#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/string/innobase_strcasecmp.h>

/** A function object to check if the foreign key constraint has the same
name as given.  If the full name of the foreign key constraint doesn't match,
then, check if removing the database name from the foreign key constraint
matches. Return true if it matches, false otherwise. */
struct dict_foreign_matches_id {
  dict_foreign_matches_id(const char *id) : m_id(id) {}

  bool operator()(const dict_foreign_t *foreign) const {
    if (0 == innobase_strcasecmp(foreign->id, m_id)) {
      return (true);
    }
    if (const char *pos = strchr(foreign->id, '/')) {
      if (0 == innobase_strcasecmp(m_id, pos + 1)) {
        return (true);
      }
    }
    return (false);
  }

  const char *m_id;
};
