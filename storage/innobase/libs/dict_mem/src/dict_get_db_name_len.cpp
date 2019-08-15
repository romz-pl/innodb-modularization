#include <innodb/dict_mem/dict_get_db_name_len.h>

#include <cstring>

/** Get the database name length in a table name.
 @return database name length */
ulint dict_get_db_name_len(const char *name) /*!< in: table name in the form
                                             dbname '/' tablename */
{
  const char *s;
  s = strchr(name, '/');
  if (s == nullptr) {
    return (0);
  }
  return (s - name);
}
