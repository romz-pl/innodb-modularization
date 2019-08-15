#include <innodb/dict_mem/dict_mem_foreign_table_name_lookup_set.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/memory/mem_heap_alloc.h>

ulint innobase_get_lower_case_table_names(void);
void innobase_casedn_str(char *a);

/** Sets the foreign_table_name_lookup pointer based on the value of
 lower_case_table_names.  If that is 0 or 1, foreign_table_name_lookup
 will point to foreign_table_name.  If 2, then another string is
 allocated from foreign->heap and set to lower case. */
void dict_mem_foreign_table_name_lookup_set(
    dict_foreign_t *foreign, /*!< in/out: foreign struct */
    ibool do_alloc)          /*!< in: is an alloc needed */
{
  if (innobase_get_lower_case_table_names() == 2) {
    if (do_alloc) {
      ulint len;

      len = strlen(foreign->foreign_table_name) + 1;

      foreign->foreign_table_name_lookup =
          static_cast<char *>(mem_heap_alloc(foreign->heap, len));
    }
    strcpy(foreign->foreign_table_name_lookup, foreign->foreign_table_name);
    innobase_casedn_str(foreign->foreign_table_name_lookup);
  } else {
    foreign->foreign_table_name_lookup = foreign->foreign_table_name;
  }
}
