#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>
#include <innodb/dict_mem/dict_vcol_set.h>

#include <iosfwd>

struct dict_table_t;
struct dict_index_t;

/** Data structure for a foreign key constraint; an example:
FOREIGN KEY (A, B) REFERENCES TABLE2 (C, D).  Most fields will be
initialized to 0, NULL or FALSE in dict_mem_foreign_create(). */
struct dict_foreign_t {
  mem_heap_t *heap;         /*!< this object is allocated from
                            this memory heap */
  char *id;                 /*!< id of the constraint as a
                            null-terminated string */
  unsigned n_fields : 10;   /*!< number of indexes' first fields
                            for which the foreign key
                            constraint is defined: we allow the
                            indexes to contain more fields than
                            mentioned in the constraint, as long
                            as the first fields are as mentioned */
  unsigned type : 6;        /*!< 0 or DICT_FOREIGN_ON_DELETE_CASCADE
                            or DICT_FOREIGN_ON_DELETE_SET_NULL */
  char *foreign_table_name; /*!< foreign table name */
  char *foreign_table_name_lookup;
  /*!< foreign table name used for dict lookup */
  dict_table_t *foreign_table;    /*!< table where the foreign key is */
  const char **foreign_col_names; /*!< names of the columns in the
                                foreign key */
  char *referenced_table_name;    /*!< referenced table name */
  char *referenced_table_name_lookup;
  /*!< referenced table name for dict lookup*/
  dict_table_t *referenced_table;    /*!< table where the referenced key
                                    is */
  const char **referenced_col_names; /*!< names of the referenced
                                columns in the referenced table */
  dict_index_t *foreign_index;       /*!< foreign index; we require that
                                     both tables contain explicitly defined
                                     indexes for the constraint: InnoDB
                                     does not generate new indexes
                                     implicitly */
  dict_index_t *referenced_index;    /*!< referenced index */

  dict_vcol_set *v_cols; /*!< set of virtual columns affected
                         by foreign key constraint. */
};

std::ostream &operator<<(std::ostream &out, const dict_foreign_t &foreign);
