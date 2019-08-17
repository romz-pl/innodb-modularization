#include <innodb/page/page_zip_write_rec.h>

#include <innodb/assert/ASSERT_ZERO.h>
#include <innodb/assert/assert.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/disk/page_t.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_find.h>
#include <innodb/page/page_zip_dir_size.h>
#include <innodb/page/page_zip_dir_start.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_simple_validate.h>
#include <innodb/page/page_zip_write_rec_ext.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_deleted_flag.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_extra_size.h>



/** Write an entire record on the compressed page.  The data must already
 have been written to the uncompressed page. */
void page_zip_write_rec(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const byte *rec,           /*!< in: record being written */
    const dict_index_t *index, /*!< in: the index the record belongs to */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint create)              /*!< in: nonzero=insert, zero=update */
{
  const page_t *page;
  byte *data;
  byte *storage;
  ulint heap_no;
  byte *slot;

  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + page_zip_dir_size(page_zip));
  ut_ad(rec_offs_comp(offsets));
  ut_ad(rec_offs_validate(rec, index, offsets));

  ut_ad(page_zip->m_start >= PAGE_DATA);

  page = page_align(rec);

  ut_ad(page_zip_header_cmp(page_zip, page));
  ut_ad(page_simple_validate_new((page_t *)page));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  slot = page_zip_dir_find(page_zip, page_offset(rec));
  ut_a(slot);
  /* Copy the delete mark. */
  if (rec_get_deleted_flag(rec, TRUE)) {
    *slot |= PAGE_ZIP_DIR_SLOT_DEL >> 8;
  } else {
    *slot &= ~(PAGE_ZIP_DIR_SLOT_DEL >> 8);
  }

  ut_ad(rec_get_start((rec_t *)rec, offsets) >= page + PAGE_ZIP_START);
  ut_ad(rec_get_end((rec_t *)rec, offsets) <=
        page + UNIV_PAGE_SIZE - PAGE_DIR -
            PAGE_DIR_SLOT_SIZE * page_dir_get_n_slots(page));

  heap_no = rec_get_heap_no_new(rec);
  ut_ad(heap_no >= PAGE_HEAP_NO_USER_LOW); /* not infimum or supremum */
  ut_ad(heap_no < page_dir_get_n_heap(page));

  /* Append to the modification log. */
  data = page_zip->data + page_zip->m_end;
  ut_ad(!*data);

  /* Identify the record by writing its heap number - 1.
  0 is reserved to indicate the end of the modification log. */

  if (UNIV_UNLIKELY(heap_no - 1 >= 64)) {
    *data++ = (byte)(0x80 | (heap_no - 1) >> 7);
    ut_ad(!*data);
  }
  *data++ = (byte)((heap_no - 1) << 1);
  ut_ad(!*data);

  {
    const byte *start = rec - rec_offs_extra_size(offsets);
    const byte *b = rec - REC_N_NEW_EXTRA_BYTES;

    /* Write the extra bytes backwards, so that
    rec_offs_extra_size() can be easily computed in
    page_zip_apply_log() by invoking
    rec_get_offsets_reverse(). */

    while (b != start) {
      *data++ = *--b;
      ut_ad(!*data);
    }
  }

  /* Write the data bytes.  Store the uncompressed bytes separately. */
  storage = page_zip_dir_start(page_zip);

  if (page_is_leaf(page)) {
    ulint len;

    if (index->is_clustered()) {
      ulint trx_id_col;

      trx_id_col = index->get_sys_col_pos(DATA_TRX_ID);
      ut_ad(trx_id_col != ULINT_UNDEFINED);

      /* Store separately trx_id, roll_ptr and
      the BTR_EXTERN_FIELD_REF of each BLOB column. */
      if (rec_offs_any_extern(offsets)) {
        data =
            page_zip_write_rec_ext(page_zip, page, rec, index, offsets, create,
                                   trx_id_col, heap_no, storage, data);
      } else {
        /* Locate trx_id and roll_ptr. */
        const byte *src = rec_get_nth_field(rec, offsets, trx_id_col, &len);
        ut_ad(len == DATA_TRX_ID_LEN);
        ut_ad(src + DATA_TRX_ID_LEN ==
              rec_get_nth_field(rec, offsets, trx_id_col + 1, &len));
        ut_ad(len == DATA_ROLL_PTR_LEN);

        /* Log the preceding fields. */
        ASSERT_ZERO(data, src - rec);
        memcpy(data, rec, src - rec);
        data += src - rec;

        /* Store trx_id and roll_ptr. */
        memcpy(storage - (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) * (heap_no - 1),
               src, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

        src += DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;

        /* Log the last bytes of the record. */
        len = rec_offs_data_size(offsets) - (src - rec);

        ASSERT_ZERO(data, len);
        memcpy(data, src, len);
        data += len;
      }
    } else {
      /* Leaf page of a secondary index:
      no externally stored columns */
      ut_ad(index->get_sys_col_pos(DATA_TRX_ID) == ULINT_UNDEFINED);
      ut_ad(!rec_offs_any_extern(offsets));

      /* Log the entire record. */
      len = rec_offs_data_size(offsets);

      ASSERT_ZERO(data, len);
      memcpy(data, rec, len);
      data += len;
    }
  } else {
    /* This is a node pointer page. */
    ulint len;

    /* Non-leaf nodes should not have any externally
    stored columns. */
    ut_ad(!rec_offs_any_extern(offsets));

    /* Copy the data bytes, except node_ptr. */
    len = rec_offs_data_size(offsets) - REC_NODE_PTR_SIZE;
    ut_ad(data + len <
          storage - REC_NODE_PTR_SIZE *
                        (page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW));
    ASSERT_ZERO(data, len);
    memcpy(data, rec, len);
    data += len;

    /* Copy the node pointer to the uncompressed area. */
    memcpy(storage - REC_NODE_PTR_SIZE * (heap_no - 1), rec + len,
           REC_NODE_PTR_SIZE);
  }

  ut_a(!*data);
  ut_ad((ulint)(data - page_zip->data) < page_zip_get_size(page_zip));
  page_zip->m_end = data - page_zip->data;
  page_zip->m_nonempty = TRUE;

#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page_align(rec), index));
#endif /* UNIV_ZIP_DEBUG */
}
