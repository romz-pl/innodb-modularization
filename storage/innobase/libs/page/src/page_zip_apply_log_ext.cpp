#include <innodb/page/page_zip_apply_log_ext.h>

#include <innodb/assert/assert.h>
#include <innodb/compiler_hints/compiler_hints.h>
#include <innodb/data_types/flags.h>
#include <innodb/disk/flags.h>
#include <innodb/page/page_zip_fail.h>
#include <innodb/record/rec_get_end.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/record/rec_offs_nth_extern.h>


/** Apply the modification log to a record containing externally stored
 columns.  Do not copy the fields that are stored separately.
 @return pointer to modification log, or NULL on failure */
const byte *page_zip_apply_log_ext(
    rec_t *rec,           /*!< in/out: record */
    const ulint *offsets, /*!< in: rec_get_offsets(rec) */
    ulint trx_id_col,     /*!< in: position of of DB_TRX_ID */
    const byte *data,     /*!< in: modification log */
    const byte *end)      /*!< in: end of modification log */
{
  ulint i;
  ulint len;
  byte *next_out = rec;

  /* Check if there are any externally stored columns.
  For each externally stored column, skip the
  BTR_EXTERN_FIELD_REF. */

  for (i = 0; i < rec_offs_n_fields(offsets); i++) {
    byte *dst;

    if (UNIV_UNLIKELY(i == trx_id_col)) {
      /* Skip trx_id and roll_ptr */
      dst = rec_get_nth_field(rec, offsets, i, &len);
      if (UNIV_UNLIKELY(dst - next_out >= end - data) ||
          UNIV_UNLIKELY(len < (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN)) ||
          rec_offs_nth_extern(offsets, i)) {
        page_zip_fail(
            ("page_zip_apply_log_ext:"
             " trx_id len %lu,"
             " %p - %p >= %p - %p\n",
             (ulong)len, (const void *)dst, (const void *)next_out,
             (const void *)end, (const void *)data));
        return (NULL);
      }

      memcpy(next_out, data, dst - next_out);
      data += dst - next_out;
      next_out = dst + (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
    } else if (rec_offs_nth_extern(offsets, i)) {
      dst = rec_get_nth_field(rec, offsets, i, &len);
      ut_ad(len >= BTR_EXTERN_FIELD_REF_SIZE);

      len += dst - next_out - BTR_EXTERN_FIELD_REF_SIZE;

      if (UNIV_UNLIKELY(data + len >= end)) {
        page_zip_fail(
            ("page_zip_apply_log_ext:"
             " ext %p+%lu >= %p\n",
             (const void *)data, (ulong)len, (const void *)end));
        return (NULL);
      }

      memcpy(next_out, data, len);
      data += len;
      next_out += len + BTR_EXTERN_FIELD_REF_SIZE;
    }
  }

  /* Copy the last bytes of the record. */
  len = rec_get_end(rec, offsets) - next_out;
  if (UNIV_UNLIKELY(data + len >= end)) {
    page_zip_fail(
        ("page_zip_apply_log_ext:"
         " last %p+%lu >= %p\n",
         (const void *)data, (ulong)len, (const void *)end));
    return (NULL);
  }
  memcpy(next_out, data, len);
  data += len;

  return (data);
}
