#include <innodb/dict_mem/dict_mem_referenced_table_name_lookup_set.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/memory/mem_heap_alloc.h>

ulint innobase_get_lower_case_table_names(void);
void innobase_casedn_str(char *a);

/** Sets the referenced_table_name_lookup pointer based on the value of
 lower_case_table_names.  If that is 0 or 1, referenced_table_name_lookup
 will point to referenced_table_name.  If 2, then another string is
 allocated from foreign->heap and set to lower case. */
void dict_mem_referenced_table_name_lookup_set(
    dict_foreign_t *foreign, /*!< in/out: foreign struct */
    ibool do_alloc)          /*!< in: is an alloc needed */
{
  if (innobase_get_lower_case_table_names() == 2) {
    if (do_alloc) {
      ulint len;

      len = strlen(foreign->referenced_table_name) + 1;

      foreign->referenced_table_name_lookup =
          static_cast<char *>(mem_heap_alloc(foreign->heap, len));
    }
    strcpy(foreign->referenced_table_name_lookup,
           foreign->referenced_table_name);
    innobase_casedn_str(foreign->referenced_table_name_lookup);
  } else {
    foreign->referenced_table_name_lookup = foreign->referenced_table_name;
  }
}
