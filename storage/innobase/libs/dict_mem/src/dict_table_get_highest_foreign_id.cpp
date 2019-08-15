#include <innodb/dict_mem/dict_table_get_highest_foreign_id.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/string/ut_strlen.h>
#include <innodb/memory/ut_memcmp.h>

/** Finds the highest [number] for foreign key constraints of the table. Looks
 only at the >= 4.0.18-format id's, which are of the form
 databasename/tablename_ibfk_[number].
 @return highest number, 0 if table has no new format foreign key constraints */
ulint dict_table_get_highest_foreign_id(
    dict_table_t *table) /*!< in: table in the dictionary memory cache */
{
  dict_foreign_t *foreign;
  char *endp;
  ulint biggest_id = 0;
  ulint id;
  ulint len;

  DBUG_ENTER("dict_table_get_highest_foreign_id");

  ut_a(table);

  len = ut_strlen(table->name.m_name);

  for (dict_foreign_set::iterator it = table->foreign_set.begin();
       it != table->foreign_set.end(); ++it) {
    foreign = *it;

    if (ut_strlen(foreign->id) > ((sizeof dict_ibfk) - 1) + len &&
        0 == ut_memcmp(foreign->id, table->name.m_name, len) &&
        0 == ut_memcmp(foreign->id + len, dict_ibfk, (sizeof dict_ibfk) - 1) &&
        foreign->id[len + ((sizeof dict_ibfk) - 1)] != '0') {
      /* It is of the >= 4.0.18 format */

      id = strtoul(foreign->id + len + ((sizeof dict_ibfk) - 1), &endp, 10);
      if (*endp == '\0') {
        ut_a(id != biggest_id);

        if (id > biggest_id) {
          biggest_id = id;
        }
      }
    }
  }

  ulint size = table->foreign_set.size();

  biggest_id = (size > biggest_id) ? size : biggest_id;

  DBUG_PRINT("dict_table_get_highest_foreign_id", ("id: %lu", biggest_id));

  DBUG_RETURN(biggest_id);
}
