#include <innodb/page/page_zip_write_rec_ext.h>

#include <innodb/assert/ASSERT_ZERO_BLOB.h>
#include <innodb/assert/ASSERT_ZERO.h>
#include <innodb/assert/assert.h>
#include <innodb/data_types/flags.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_get_n_prev_extern.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_n_extern.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/record/rec_get_nth_field.h>


/** Write a record on the compressed page that contains externally stored
 columns.  The data must already have been written to the uncompressed page.
 @return end of modification log */
byte *page_zip_write_rec_ext(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const page_t *page,        /*!< in: page containing rec */
    const byte *rec,           /*!< in: record being written */
    const dict_index_t *index, /*!< in: record descriptor */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint create,              /*!< in: nonzero=insert, zero=update */
    ulint trx_id_col,          /*!< in: position of DB_TRX_ID */
    ulint heap_no,             /*!< in: heap number of rec */
    byte *storage,             /*!< in: end of dense page directory */
    byte *data)                /*!< in: end of modification log */
{
  const byte *start = rec;
  ulint i;
  ulint len;
  byte *externs = storage;
  ulint n_ext = rec_offs_n_extern(offsets);

  ut_ad(rec_offs_validate(rec, index, offsets));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  externs -= (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) *
             (page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW);

  /* Note that this will not take into account
  the BLOB columns of rec if create==TRUE. */
  ut_ad(data + rec_offs_data_size(offsets) -
            (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) -
            n_ext * BTR_EXTERN_FIELD_REF_SIZE <
        externs - BTR_EXTERN_FIELD_REF_SIZE * page_zip->n_blobs);

  {
    ulint blob_no = page_zip_get_n_prev_extern(page_zip, rec, index);
    byte *ext_end = externs - page_zip->n_blobs * BTR_EXTERN_FIELD_REF_SIZE;
    ut_ad(blob_no <= page_zip->n_blobs);
    externs -= blob_no * BTR_EXTERN_FIELD_REF_SIZE;

    if (create) {
      page_zip->n_blobs += static_cast<unsigned>(n_ext);
      ASSERT_ZERO_BLOB(ext_end - n_ext * BTR_EXTERN_FIELD_REF_SIZE);
      memmove(ext_end - n_ext * BTR_EXTERN_FIELD_REF_SIZE, ext_end,
              externs - ext_end);
    }

    ut_a(blob_no + n_ext <= page_zip->n_blobs);
  }

  for (i = 0; i < rec_offs_n_fields(offsets); i++) {
    const byte *src;

    if (UNIV_UNLIKELY(i == trx_id_col)) {
      ut_ad(!rec_offs_nth_extern(offsets, i));
      ut_ad(!rec_offs_nth_extern(offsets, i + 1));
      /* Locate trx_id and roll_ptr. */
      src = rec_get_nth_field(rec, offsets, i, &len);
      ut_ad(len == DATA_TRX_ID_LEN);
      ut_ad(src + DATA_TRX_ID_LEN ==
            rec_get_nth_field(rec, offsets, i + 1, &len));
      ut_ad(len == DATA_ROLL_PTR_LEN);

      /* Log the preceding fields. */
      ASSERT_ZERO(data, src - start);
      memcpy(data, start, src - start);
      data += src - start;
      start = src + (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

      /* Store trx_id and roll_ptr. */
      memcpy(storage - (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) * (heap_no - 1),
             src, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
      i++; /* skip also roll_ptr */
    } else if (rec_offs_nth_extern(offsets, i)) {
      src = rec_get_nth_field(rec, offsets, i, &len);

      ut_ad(index->is_clustered());
      ut_ad(len >= BTR_EXTERN_FIELD_REF_SIZE);
      src += len - BTR_EXTERN_FIELD_REF_SIZE;

      ASSERT_ZERO(data, src - start);
      memcpy(data, start, src - start);
      data += src - start;
      start = src + BTR_EXTERN_FIELD_REF_SIZE;

      /* Store the BLOB pointer. */
      externs -= BTR_EXTERN_FIELD_REF_SIZE;
      ut_ad(data < externs);
      memcpy(externs, src, BTR_EXTERN_FIELD_REF_SIZE);
    }
  }

  /* Log the last bytes of the record. */
  len = rec_offs_data_size(offsets) - (start - rec);

  ASSERT_ZERO(data, len);
  memcpy(data, start, len);
  data += len;

  return (data);
}
