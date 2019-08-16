#include <innodb/record/rec_get_converted_size_comp_prefix_low.h>

#include <innodb/bit/UT_BITS_IN_BYTES.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dtuple_get_n_v_fields.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/dict_mem/DICT_MAX_FIELD_LEN_BY_FORMAT.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_v_col_t.h>
#include <innodb/machine/data.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_n_fields_length.h>


/** Determines the size of a data tuple prefix in ROW_FORMAT=COMPACT.
 @return total size */
UNIV_INLINE MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_get_converted_size_comp_prefix_low(
        const dict_index_t *index, /*!< in: record descriptor;
                                   dict_table_is_comp() is
                                   assumed to hold, even if
                                   it does not */
        const dfield_t *fields,    /*!< in: array of data fields */
        ulint n_fields,            /*!< in: number of data fields */
        const dtuple_t *v_entry,   /*!< in: dtuple contains virtual column
                                   data */
        ulint *extra,              /*!< out: extra size */
        ulint *status,             /*!< in: status bits of the record,
                                   can be nullptr if unnecessary */
        bool temp)                 /*!< in: whether this is a
                                   temporary file record */
{
  ulint extra_size = 0;
  ulint data_size;
  ulint i;
  ulint n_null = 0;
  ulint n_v_fields;
  ut_ad(n_fields <= dict_index_get_n_fields(index));
  ut_ad(!temp || extra);

  /* At the time being, only temp file record could possible
  store virtual columns */
  ut_ad(!v_entry || (index->is_clustered() && temp));
  n_v_fields = v_entry ? dtuple_get_n_v_fields(v_entry) : 0;

  if (n_fields > 0) {
    n_null = index->has_instant_cols()
                 ? index->get_n_nullable_before(static_cast<uint32_t>(n_fields))
                 : index->n_nullable;
  }

  if (index->has_instant_cols() && status != nullptr) {
    switch (UNIV_EXPECT(*status, REC_STATUS_ORDINARY)) {
      case REC_STATUS_ORDINARY:
        ut_ad(!temp && n_fields > 0);
        extra_size += rec_get_n_fields_length(n_fields);
        break;
      case REC_STATUS_NODE_PTR:
        ut_ad(!temp && n_fields > 0);
        n_null = index->n_instant_nullable;
        break;
      case REC_STATUS_INFIMUM:
      case REC_STATUS_SUPREMUM:
        break;
    }
  }

  extra_size += temp ? UT_BITS_IN_BYTES(n_null)
                     : REC_N_NEW_EXTRA_BYTES + UT_BITS_IN_BYTES(n_null);
  data_size = 0;

  if (temp && dict_table_is_comp(index->table)) {
    /* No need to do adjust fixed_len=0. We only need to
    adjust it for ROW_FORMAT=REDUNDANT. */
    temp = false;
  }

  /* read the lengths of fields 0..n */
  for (i = 0; i < n_fields; i++) {
    const dict_field_t *field;
    ulint len;
    ulint fixed_len;
    const dict_col_t *col;

    field = index->get_field(i);
    len = dfield_get_len(&fields[i]);
    col = field->col;

#ifdef UNIV_DEBUG
    dtype_t *type;

    type = dfield_get_type(&fields[i]);
    if (dict_index_is_spatial(index)) {
      if (DATA_GEOMETRY_MTYPE(col->mtype) && i == 0) {
        ut_ad(type->prtype & DATA_GIS_MBR);
      } else {
        ut_ad(type->mtype == DATA_SYS_CHILD || col->assert_equal(type));
      }
    } else {
      ut_ad(col->assert_equal(type));
    }
#endif

    /* All NULLable fields must be included in the n_null count. */
    ut_ad((col->prtype & DATA_NOT_NULL) || n_null--);

    if (dfield_is_null(&fields[i])) {
      /* No length is stored for NULL fields. */
      ut_ad(!(col->prtype & DATA_NOT_NULL));
      continue;
    }

    ut_ad(len <= col->len || DATA_LARGE_MTYPE(col->mtype) ||
          (DATA_POINT_MTYPE(col->mtype) && len == DATA_MBR_LEN) ||
          (col->len == 0 && col->mtype == DATA_VARCHAR));

    fixed_len = field->fixed_len;
    if (temp && fixed_len && !col->get_fixed_size(temp)) {
      fixed_len = 0;
    }
    /* If the maximum length of a variable-length field
    is up to 255 bytes, the actual length is always stored
    in one byte. If the maximum length is more than 255
    bytes, the actual length is stored in one byte for
    0..127.  The length will be encoded in two bytes when
    it is 128 or more, or when the field is stored externally. */

    if (fixed_len) {
#ifdef UNIV_DEBUG
      ulint mbminlen = DATA_MBMINLEN(col->mbminmaxlen);
      ulint mbmaxlen = DATA_MBMAXLEN(col->mbminmaxlen);

      ut_ad(len <= fixed_len);

      if (dict_index_is_spatial(index)) {
        ut_ad(type->mtype == DATA_SYS_CHILD || !mbmaxlen ||
              len >= mbminlen * (fixed_len / mbmaxlen));
      } else {
        ut_ad(type->mtype != DATA_SYS_CHILD);
        ut_ad(!mbmaxlen || len >= mbminlen * (fixed_len / mbmaxlen));
      }

      /* dict_index_add_col() should guarantee this */
      ut_ad(!field->prefix_len || fixed_len == field->prefix_len);
#endif /* UNIV_DEBUG */
    } else if (dfield_is_ext(&fields[i])) {
      ut_ad(DATA_BIG_COL(col));
      extra_size += 2;
    } else if (len < 128 || !DATA_BIG_COL(col)) {
      extra_size++;
    } else {
      /* For variable-length columns, we look up the
      maximum length from the column itself.  If this
      is a prefix index column shorter than 256 bytes,
      this will waste one byte. */
      extra_size += 2;
    }
    data_size += len;
  }

  if (extra) {
    *extra = extra_size;
  }

  /* Log virtual columns */
  if (n_v_fields != 0) {
    /* length marker */
    data_size += 2;

    for (i = 0; i < n_v_fields; i++) {
      dfield_t *vfield;
      ulint flen;

      const dict_v_col_t *col = dict_table_get_nth_v_col(index->table, i);

      /* Only those indexed needs to be logged */
      if (col->m_col.ord_part || !dict_table_is_comp(index->table)) {
        data_size += mach_get_compressed_size(i + REC_MAX_N_FIELDS);
        vfield = dtuple_get_nth_v_field(v_entry, col->v_pos);

        flen = vfield->len;

        if (flen != UNIV_SQL_NULL) {
          flen = std::min(flen, static_cast<ulint>(
                                  DICT_MAX_FIELD_LEN_BY_FORMAT(index->table)));
          data_size += flen;
        }

        data_size += mach_get_compressed_size(flen);
      }
    }
  }

  return (extra_size + data_size);
}
