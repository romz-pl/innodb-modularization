#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Gets the number of user-defined ordering fields in the index. In the
 internal representation of clustered indexes we add the row id to the ordering
 fields to make a clustered index unique, but this function returns the number
 of fields the user defined in the index as ordering fields.
 @return number of fields */
UNIV_INLINE
ulint dict_index_get_n_ordering_defined_by_user(
    const dict_index_t *index) /*!< in: an internal representation
                               of index (in the dictionary cache) */
{
  return (index->n_user_defined_cols);
}
