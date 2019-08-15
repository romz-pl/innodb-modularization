#pragma once

#include <innodb/univ/univ.h>

struct dict_foreign_t;

/** Sets the referenced_table_name_lookup pointer based on the value of
 lower_case_table_names.  If that is 0 or 1, referenced_table_name_lookup
 will point to referenced_table_name.  If 2, then another string is
 allocated from the heap and set to lower case. */
void dict_mem_referenced_table_name_lookup_set(
    dict_foreign_t *foreign, /*!< in/out: foreign struct */
    ibool do_alloc);         /*!< in: is an alloc needed */
