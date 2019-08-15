#include <innodb/dict_mem/dict_remove_db_name.h>

#include <innodb/assert/assert.h>

#include <cstring>

/** Return the end of table name where we have removed dbname and '/'.
 @return table name */
const char *dict_remove_db_name(
    const char *name) /*!< in: table name in the form
                      dbname '/' tablename */
{
  const char *s = strchr(name, '/');
  ut_a(s);

  return (s + 1);
}
