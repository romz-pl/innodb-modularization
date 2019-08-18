#include <innodb/page/page_zip_dir_delete.h>

#include <innodb/assert/assert.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/disk/page_t.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_header_set_field.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_zip_clear_rec.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_find.h>
#include <innodb/page/page_zip_dir_find_free.h>
#include <innodb/page/page_zip_get_n_prev_extern.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/record/rec_offs_n_extern.h>


/** Shift the dense page directory when a record is deleted.
@param[in,out]	page_zip	compressed page
@param[in]	rec		deleted record
@param[in]	index		index of rec
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	free		previous start of the free list */
void page_zip_dir_delete(page_zip_des_t *page_zip, byte *rec,
                         const dict_index_t *index, const ulint *offsets,
                         const byte *free) {
  byte *slot_rec;
  byte *slot_free;
  ulint n_ext;
  page_t *page = page_align(rec);

  ut_ad(rec_offs_validate(rec, index, offsets));
  ut_ad(rec_offs_comp(offsets));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  slot_rec = page_zip_dir_find(page_zip, page_offset(rec));

  ut_a(slot_rec);

  /* This could not be done before page_zip_dir_find(). */
  page_header_set_field(page, page_zip, PAGE_N_RECS,
                        (ulint)(page_get_n_recs(page) - 1));

  if (UNIV_UNLIKELY(!free)) {
    /* Make the last slot the start of the free list. */
    slot_free = page_zip->data + page_zip_get_size(page_zip) -
                PAGE_ZIP_DIR_SLOT_SIZE * (page_dir_get_n_heap(page_zip->data) -
                                          PAGE_HEAP_NO_USER_LOW);
  } else {
    slot_free = page_zip_dir_find_free(page_zip, page_offset(free));
    ut_a(slot_free < slot_rec);
    /* Grow the free list by one slot by moving the start. */
    slot_free += PAGE_ZIP_DIR_SLOT_SIZE;
  }

  if (UNIV_LIKELY(slot_rec > slot_free)) {
    memmove(slot_free + PAGE_ZIP_DIR_SLOT_SIZE, slot_free,
            slot_rec - slot_free);
  }

  /* Write the entry for the deleted record.
  The "owned" and "deleted" flags will be cleared. */
  mach_write_to_2(slot_free, page_offset(rec));

  if (!page_is_leaf(page) || !index->is_clustered()) {
    ut_ad(!rec_offs_any_extern(offsets));
    goto skip_blobs;
  }

  n_ext = rec_offs_n_extern(offsets);
  if (UNIV_UNLIKELY(n_ext)) {
    /* Shift and zero fill the array of BLOB pointers. */
    ulint blob_no;
    byte *externs;
    byte *ext_end;

    blob_no = page_zip_get_n_prev_extern(page_zip, rec, index);
    ut_a(blob_no + n_ext <= page_zip->n_blobs);

    externs = page_zip->data + page_zip_get_size(page_zip) -
              (page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW) *
                  PAGE_ZIP_CLUST_LEAF_SLOT_SIZE;

    ext_end = externs - page_zip->n_blobs * BTR_EXTERN_FIELD_REF_SIZE;
    externs -= blob_no * BTR_EXTERN_FIELD_REF_SIZE;

    page_zip->n_blobs -= static_cast<unsigned>(n_ext);
    /* Shift and zero fill the array. */
    memmove(ext_end + n_ext * BTR_EXTERN_FIELD_REF_SIZE, ext_end,
            (page_zip->n_blobs - blob_no) * BTR_EXTERN_FIELD_REF_SIZE);
    memset(ext_end, 0, n_ext * BTR_EXTERN_FIELD_REF_SIZE);
  }

skip_blobs:
  /* The compression algorithm expects info_bits and n_owned
  to be 0 for deleted records. */
  rec[-REC_N_NEW_EXTRA_BYTES] = 0; /* info_bits and n_owned */

  page_zip_clear_rec(page_zip, rec, index, offsets);
}
