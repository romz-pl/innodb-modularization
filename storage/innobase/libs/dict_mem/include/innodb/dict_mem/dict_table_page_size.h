#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_tf_get_page_size.h>

/** Get the table page size.
@param[in]	table	table
@return a structure containing the compressed and uncompressed
page sizes and a boolean indicating if the page is compressed */
UNIV_INLINE
const page_size_t dict_table_page_size(const dict_table_t *table) {
  ut_ad(table != NULL);

  return (dict_tf_get_page_size(table->flags));
}
