#include <innodb/page/page_zip_get_n_prev_extern.h>

#include <innodb/assert/assert.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_get.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_n_extern_new.h>

/** Determine how many externally stored columns are contained
 in existing records with smaller heap_no than rec. */
ulint page_zip_get_n_prev_extern(
    const page_zip_des_t *page_zip, /*!< in: dense page directory on
                                   compressed page */
    const rec_t *rec,               /*!< in: compact physical record
                                    on a B-tree leaf page */
    const dict_index_t *index)      /*!< in: record descriptor */
{
  const page_t *page = page_align(rec);
  ulint n_ext = 0;
  ulint i;
  ulint left;
  ulint heap_no;
  ulint n_recs = page_get_n_recs(page_zip->data);

  ut_ad(page_is_leaf(page));
  ut_ad(page_is_comp(page));
  ut_ad(dict_table_is_comp(index->table));
  ut_ad(index->is_clustered());
  ut_ad(!dict_index_is_ibuf(index));

  heap_no = rec_get_heap_no_new(rec);
  ut_ad(heap_no >= PAGE_HEAP_NO_USER_LOW);
  left = heap_no - PAGE_HEAP_NO_USER_LOW;
  if (UNIV_UNLIKELY(!left)) {
    return (0);
  }

  for (i = 0; i < n_recs; i++) {
    const rec_t *r =
        page + (page_zip_dir_get(page_zip, i) & PAGE_ZIP_DIR_SLOT_MASK);

    if (rec_get_heap_no_new(r) < heap_no) {
      n_ext += rec_get_n_extern_new(r, index, ULINT_UNDEFINED);
      if (!--left) {
        break;
      }
    }
  }

  return (n_ext);
}

