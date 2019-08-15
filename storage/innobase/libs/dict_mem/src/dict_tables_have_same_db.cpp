#include <innodb/dict_mem/dict_tables_have_same_db.h>

#include <innodb/assert/assert.h>

/** Checks if the database name in two table names is the same.
 @return true if same db name */
ibool dict_tables_have_same_db(
    const char *name1, /*!< in: table name in the form
                       dbname '/' tablename */
    const char *name2) /*!< in: table name in the form
                       dbname '/' tablename */
{
  for (; *name1 == *name2; name1++, name2++) {
    if (*name1 == '/') {
      return (TRUE);
    }
    ut_a(*name1); /* the names must contain '/' */
  }
  return (FALSE);
}
