#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Gets the page number of the root of the index tree.
 @return page number */
UNIV_INLINE
page_no_t dict_index_get_page(const dict_index_t *index) /*!< in: index */
{
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  return (index->page);
}
