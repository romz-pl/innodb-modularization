#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_init_null_and_len_temp.h>
#include <innodb/record/rec_init_null_and_len_comp.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/record/rec_get_instant_offset.h>
#include <innodb/record/rec_offs_n_fields.h>

/** Determine the offset to each field in a leaf-page record
 in ROW_FORMAT=COMPACT.  This is a special case of
 rec_init_offsets() and rec_get_offsets_func(). */
UNIV_INLINE
void rec_init_offsets_comp_ordinary(
    const rec_t *rec,          /*!< in: physical record in
                               ROW_FORMAT=COMPACT */
    bool temp,                 /*!< in: whether to use the
                               format for temporary files in
                               index creation */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets)            /*!< in/out: array of offsets;
                               in: n=rec_offs_n_fields(offsets) */
{
  uint16_t i = 0;
  ulint offs = 0;
  ulint any_ext = 0;
  uint16_t n_null = 0;
  const byte *nulls = nullptr;
  const byte *lens = nullptr;
  ulint null_mask = 1;
  uint16_t non_default_fields = 0;

#ifdef UNIV_DEBUG
  /* We cannot invoke rec_offs_make_valid() here if temp=true.
  Similarly, rec_offs_validate() will fail in that case, because
  it invokes rec_get_status(). */
  offsets[2] = (ulint)rec;
  offsets[3] = (ulint)index;
#endif /* UNIV_DEBUG */

  if (temp) {
    non_default_fields =
        rec_init_null_and_len_temp(rec, index, &nulls, &lens, &n_null);
  } else {
    non_default_fields =
        rec_init_null_and_len_comp(rec, index, &nulls, &lens, &n_null);
  }

  ut_ad(temp || dict_table_is_comp(index->table));

  if (temp && dict_table_is_comp(index->table)) {
    /* No need to do adjust fixed_len=0. We only need to
    adjust it for ROW_FORMAT=REDUNDANT. */
    temp = false;
  }

  /* read the lengths of fields 0..n */
  do {
    const dict_field_t *field = index->get_field(i);
    const dict_col_t *col = field->col;
    uint64_t len;

    if (i >= non_default_fields) {
      ut_ad(index->has_instant_cols());

      len = rec_get_instant_offset(index, i, offs);

      goto resolved;
    }

    if (!(col->prtype & DATA_NOT_NULL)) {
      /* nullable field => read the null flag */
      ut_ad(n_null--);

      if (UNIV_UNLIKELY(!(byte)null_mask)) {
        nulls--;
        null_mask = 1;
      }

      if (*nulls & null_mask) {
        null_mask <<= 1;
        /* No length is stored for NULL fields.
        We do not advance offs, and we set
        the length to zero and enable the
        SQL NULL flag in offsets[]. */
        len = offs | REC_OFFS_SQL_NULL;
        goto resolved;
      }
      null_mask <<= 1;
    }

    if (!field->fixed_len || (temp && !col->get_fixed_size(temp))) {
      ut_ad(col->mtype != DATA_POINT);
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
          len <<= 8;
          len |= *lens--;

          offs += len & 0x3fff;
          if (UNIV_UNLIKELY(len & 0x4000)) {
            ut_ad(index->is_clustered());
            any_ext = REC_OFFS_EXTERNAL;
            len = offs | REC_OFFS_EXTERNAL;
          } else {
            len = offs;
          }

          goto resolved;
        }
      }

      len = offs += len;
    } else {
      len = offs += field->fixed_len;
    }
  resolved:
    rec_offs_base(offsets)[i + 1] = len;
  } while (++i < rec_offs_n_fields(offsets));

  *rec_offs_base(offsets) = (rec - (lens + 1)) | REC_OFFS_COMPACT | any_ext;
}
