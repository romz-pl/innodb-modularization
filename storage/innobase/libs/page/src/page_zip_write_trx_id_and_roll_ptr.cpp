#include <innodb/page/page_zip_write_trx_id_and_roll_ptr.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/flags.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_start.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_nth_field.h>

/** Write the trx_id and roll_ptr of a record on a B-tree leaf node page. */
void page_zip_write_trx_id_and_roll_ptr(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    byte *rec,                /*!< in/out: record */
    const ulint *offsets,     /*!< in: rec_get_offsets(rec, index) */
    ulint trx_id_col,         /*!< in: column number of TRX_ID in rec */
    trx_id_t trx_id,          /*!< in: transaction identifier */
    roll_ptr_t roll_ptr)      /*!< in: roll_ptr */
{
  byte *field;
  byte *storage;
#ifdef UNIV_DEBUG
  page_t *page = page_align(rec);
#endif /* UNIV_DEBUG */
  ulint len;

  ut_ad(page_simple_validate_new(page));
  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + page_zip_dir_size(page_zip));
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  ut_ad(rec_offs_comp(offsets));

  ut_ad(page_zip->m_start >= PAGE_DATA);
  ut_ad(page_zip_header_cmp(page_zip, page));

  ut_ad(page_is_leaf(page));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  storage =
      page_zip_dir_start(page_zip) -
      (rec_get_heap_no_new(rec) - 1) * (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

  field = const_cast<byte *>(rec_get_nth_field(rec, offsets, trx_id_col, &len));
  ut_ad(len == DATA_TRX_ID_LEN);
  ut_ad(field + DATA_TRX_ID_LEN ==
        rec_get_nth_field(rec, offsets, trx_id_col + 1, &len));
  ut_ad(len == DATA_ROLL_PTR_LEN);
#if defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG
  ut_a(!memcmp(storage, field, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN));
#endif /* UNIV_DEBUG || UNIV_ZIP_DEBUG */
  mach_write_to_6(field, trx_id);
  mach_write_to_7(field + DATA_TRX_ID_LEN, roll_ptr);
  memcpy(storage, field, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
}
