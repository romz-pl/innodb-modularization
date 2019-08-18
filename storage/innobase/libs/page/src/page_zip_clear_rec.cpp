#include <innodb/page/page_zip_clear_rec.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/disk/page_t.h>

#include <innodb/dict_mem/dict_col_get_clust_pos.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_start.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>

/** Clear an area on the uncompressed and compressed page.
 Do not clear the data payload, as that would grow the modification log. */
void page_zip_clear_rec(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    byte *rec,                 /*!< in: record to clear */
    const dict_index_t *index, /*!< in: index of rec */
    const ulint *offsets)      /*!< in: rec_get_offsets(rec, index) */
{
  ulint heap_no;
  page_t *page = page_align(rec);
  byte *storage;
  byte *field;
  ulint len;
  /* page_zip_validate() would fail here if a record
  containing externally stored columns is being deleted. */
  ut_ad(rec_offs_validate(rec, index, offsets));
  ut_ad(!page_zip_dir_find(page_zip, page_offset(rec)));
  ut_ad(page_zip_dir_find_free(page_zip, page_offset(rec)));
  ut_ad(page_zip_header_cmp(page_zip, page));

  heap_no = rec_get_heap_no_new(rec);
  ut_ad(heap_no >= PAGE_HEAP_NO_USER_LOW);

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  if (!page_is_leaf(page)) {
    /* Clear node_ptr. On the compressed page,
    there is an array of node_ptr immediately before the
    dense page directory, at the very end of the page. */
    storage = page_zip_dir_start(page_zip);
    ut_ad(dict_index_get_n_unique_in_tree_nonleaf(index) ==
          rec_offs_n_fields(offsets) - 1);
    field =
        rec_get_nth_field(rec, offsets, rec_offs_n_fields(offsets) - 1, &len);
    ut_ad(len == REC_NODE_PTR_SIZE);

    ut_ad(!rec_offs_any_extern(offsets));
    memset(field, 0, REC_NODE_PTR_SIZE);
    memset(storage - (heap_no - 1) * REC_NODE_PTR_SIZE, 0, REC_NODE_PTR_SIZE);
  } else if (index->is_clustered()) {
    /* Clear trx_id and roll_ptr. On the compressed page,
    there is an array of these fields immediately before the
    dense page directory, at the very end of the page. */
    const ulint trx_id_pos =
        dict_col_get_clust_pos(index->table->get_sys_col(DATA_TRX_ID), index);
    storage = page_zip_dir_start(page_zip);
    field = rec_get_nth_field(rec, offsets, trx_id_pos, &len);
    ut_ad(len == DATA_TRX_ID_LEN);

    memset(field, 0, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
    memset(storage - (heap_no - 1) * (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN), 0,
           DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

    if (rec_offs_any_extern(offsets)) {
      ulint i;

      for (i = rec_offs_n_fields(offsets); i--;) {
        /* Clear all BLOB pointers in order to make
        page_zip_validate() pass. */
        if (rec_offs_nth_extern(offsets, i)) {
          field = rec_get_nth_field(rec, offsets, i, &len);
          ut_ad(len == BTR_EXTERN_FIELD_REF_SIZE);
          memset(field + len - BTR_EXTERN_FIELD_REF_SIZE, 0,
                 BTR_EXTERN_FIELD_REF_SIZE);
        }
      }
    }
  } else {
    ut_ad(!rec_offs_any_extern(offsets));
  }

#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page, index));
#endif /* UNIV_ZIP_DEBUG */
}

