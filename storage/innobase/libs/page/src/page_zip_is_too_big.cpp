#include <innodb/page/page_zip_is_too_big.h>

#include <innodb/page/page_zip_empty_size.h>
#include <innodb/dict_mem/dict_table_page_size.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree.h>
#include <innodb/record/rec_get_converted_size_comp_prefix.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/record/flag.h>



/** Check whether a tuple is too big for compressed table
@param[in]	index	dict index object
@param[in]	entry	entry for the index
@return	true if it's too big, otherwise false */
bool page_zip_is_too_big(const dict_index_t *index, const dtuple_t *entry) {
  const page_size_t &page_size = dict_table_page_size(index->table);

  /* Estimate the free space of an empty compressed page.
  Subtract one byte for the encoded heap_no in the
  modification log. */
  ulint free_space_zip =
      page_zip_empty_size(index->n_fields, page_size.physical());
  ulint n_uniq = dict_index_get_n_unique_in_tree(index);

  ut_ad(dict_table_is_comp(index->table));
  ut_ad(page_size.is_compressed());

  if (free_space_zip == 0) {
    return (true);
  }

  /* Subtract one byte for the encoded heap_no in the
  modification log. */
  free_space_zip--;

  /* There should be enough room for two node pointer
  records on an empty non-leaf page.  This prevents
  infinite page splits. */

  if (entry->n_fields >= n_uniq && (REC_NODE_PTR_SIZE +
                                        rec_get_converted_size_comp_prefix(
                                            index, entry->fields, n_uniq, NULL)
                                        /* On a compressed page, there is
                                        a two-byte entry in the dense
                                        page directory for every record.
                                        But there is no record header. */
                                        - (REC_N_NEW_EXTRA_BYTES - 2) >
                                    free_space_zip / 2)) {
    return (true);
  }

  return (false);
}


