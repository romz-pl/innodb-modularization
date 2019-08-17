#include <innodb/page/page_zip_fields_encode.h>

#include <innodb/dict_mem/dict_field_t.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_zip_fixed_field_encode.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_col_t.h>


/** Write the index information for the compressed page.
 @return used size of buf */
ulint page_zip_fields_encode(
    ulint n,                   /*!< in: number of fields
                               to compress */
    const dict_index_t *index, /*!< in: index comprising
                               at least n fields */
    ulint trx_id_pos,
    /*!< in: position of the trx_id column
    in the index, or ULINT_UNDEFINED if
    this is a non-leaf page */
    byte *buf) /*!< out: buffer of (n + 1) * 2 bytes */
{
  const byte *buf_start = buf;
  ulint i;
  ulint col;
  ulint trx_id_col = 0;
  /* sum of lengths of preceding non-nullable fixed fields, or 0 */
  ulint fixed_sum = 0;

  ut_ad(trx_id_pos == ULINT_UNDEFINED || trx_id_pos < n);

  for (i = col = 0; i < n; i++) {
    dict_field_t *field = index->get_field(i);
    ulint val;

    if (field->col->prtype & DATA_NOT_NULL) {
      val = 1; /* set the "not nullable" flag */
    } else {
      val = 0; /* nullable field */
    }

    if (!field->fixed_len) {
      /* variable-length field */
      const dict_col_t *column = field->col;

      if (DATA_BIG_COL(column)) {
        val |= 0x7e; /* max > 255 bytes */
      }

      if (fixed_sum) {
        /* write out the length of any
        preceding non-nullable fields */
        buf = page_zip_fixed_field_encode(buf, fixed_sum << 1 | 1);
        fixed_sum = 0;
        col++;
      }

      *buf++ = (byte)val;
      col++;
    } else if (val) {
      /* fixed-length non-nullable field */

      if (fixed_sum && UNIV_UNLIKELY(fixed_sum + field->fixed_len >
                                     DICT_MAX_FIXED_COL_LEN)) {
        /* Write out the length of the
        preceding non-nullable fields,
        to avoid exceeding the maximum
        length of a fixed-length column. */
        buf = page_zip_fixed_field_encode(buf, fixed_sum << 1 | 1);
        fixed_sum = 0;
        col++;
      }

      if (i && UNIV_UNLIKELY(i == trx_id_pos)) {
        if (fixed_sum) {
          /* Write out the length of any
          preceding non-nullable fields,
          and start a new trx_id column. */
          buf = page_zip_fixed_field_encode(buf, fixed_sum << 1 | 1);
          col++;
        }

        trx_id_col = col;
        fixed_sum = field->fixed_len;
      } else {
        /* add to the sum */
        fixed_sum += field->fixed_len;
      }
    } else {
      /* fixed-length nullable field */

      if (fixed_sum) {
        /* write out the length of any
        preceding non-nullable fields */
        buf = page_zip_fixed_field_encode(buf, fixed_sum << 1 | 1);
        fixed_sum = 0;
        col++;
      }

      buf = page_zip_fixed_field_encode(buf, field->fixed_len << 1);
      col++;
    }
  }

  if (fixed_sum) {
    /* Write out the lengths of last fixed-length columns. */
    buf = page_zip_fixed_field_encode(buf, fixed_sum << 1 | 1);
  }

  if (trx_id_pos != ULINT_UNDEFINED) {
    /* Write out the position of the trx_id column */
    i = trx_id_col;
  } else {
    /* Write out the number of nullable fields */
    i = index->n_nullable;
  }

  if (i < 128) {
    *buf++ = (byte)i;
  } else {
    *buf++ = (byte)(0x80 | i >> 8);
    *buf++ = (byte)i;
  }

  ut_ad((ulint)(buf - buf_start) <= (n + 2) * 2);
  return ((ulint)(buf - buf_start));
}

