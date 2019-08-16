#include <innodb/record/rec_get_n_extern_new.h>

#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/bit/UT_BITS_IN_BYTES.h>

/** Determine how many of the first n columns in a compact
 physical record are stored externally.
 @return number of externally stored columns */
ulint rec_get_n_extern_new(
    const rec_t *rec,          /*!< in: compact physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n)                   /*!< in: number of columns to scan */
{
  const byte *nulls;
  const byte *lens;
  ulint null_mask;
  ulint n_extern;
  ulint i;

  ut_ad(dict_table_is_comp(index->table));
  ut_ad(rec_get_status(rec) == REC_STATUS_ORDINARY);
  ut_ad(n == ULINT_UNDEFINED || n <= dict_index_get_n_fields(index));

  if (n == ULINT_UNDEFINED) {
    n = dict_index_get_n_fields(index);
  }

  nulls = rec - (REC_N_NEW_EXTRA_BYTES + 1);
  lens = nulls - UT_BITS_IN_BYTES(index->n_nullable);
  null_mask = 1;
  n_extern = 0;
  i = 0;

  /* read the lengths of fields 0..n */
  do {
    const dict_field_t *field = index->get_field(i);
    const dict_col_t *col = field->col;
    ulint len;

    if (!(col->prtype & DATA_NOT_NULL)) {
      /* nullable field => read the null flag */

      if (UNIV_UNLIKELY(!(byte)null_mask)) {
        nulls--;
        null_mask = 1;
      }

      if (*nulls & null_mask) {
        null_mask <<= 1;
        /* No length is stored for NULL fields. */
        continue;
      }
      null_mask <<= 1;
    }

    if (UNIV_UNLIKELY(!field->fixed_len)) {
      /* Variable-length field: read the length */
      len = *lens--;
      /* If the maximum length of the field is up
      to 255 bytes, the actual length is always
      stored in one byte. If the maximum length is
      more than 255 bytes, the actual length is
      stored in one byte for 0..127.  The length
      will be encoded in two bytes when it is 128 or
      more, or when the field is stored externally. */
      if (DATA_BIG_COL(col)) {
        if (len & 0x80) {
          /* 1exxxxxxx xxxxxxxx */
          if (len & 0x40) {
            n_extern++;
          }
          lens--;
        }
      }
    }
  } while (++i < n);

  return (n_extern);
}
