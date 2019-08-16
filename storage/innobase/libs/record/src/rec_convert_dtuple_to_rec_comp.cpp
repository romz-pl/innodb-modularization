#include <innodb/record/rec_convert_dtuple_to_rec_comp.h>

#include <innodb/bit/UT_BITS_IN_BYTES.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dtuple_get_n_v_fields.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtype_get_len.h>
#include <innodb/data_types/dtype_get_mtype.h>
#include <innodb/data_types/dtype_get_prtype.h>
#include <innodb/dict_mem/DICT_MAX_FIELD_LEN_BY_FORMAT.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_v_col_t.h>
#include <innodb/machine/data.h>
#include <innodb/memory/ut_memcpy.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_set_n_fields.h>

/** Builds a ROW_FORMAT=COMPACT record out of a data tuple.
@param[in]	rec		origin of record
@param[in]	index		record descriptor
@param[in]	fields		array of data fields
@param[in]	n_fields	number of data fields
@param[in]	v_entry		dtuple contains virtual column data
@param[in]	status		status bits of the record
@param[in]	temp		whether to use the format for temporary
                                files in index creation
@return	true	if this record is an instant record on leaf page
@retval	false	if not an instant record */
bool rec_convert_dtuple_to_rec_comp(rec_t *rec, const dict_index_t *index,
                                    const dfield_t *fields, ulint n_fields,
                                    const dtuple_t *v_entry, ulint status,
                                    bool temp) {
  const dfield_t *field;
  const dtype_t *type;
  byte *end;
  byte *nulls;
  byte *lens = NULL;
  ulint len;
  ulint i;
  ulint n_node_ptr_field;
  ulint fixed_len;
  ulint null_mask = 1;
  ulint n_null = 0;
  ulint num_v = v_entry ? dtuple_get_n_v_fields(v_entry) : 0;
  bool instant = false;

  ut_ad(temp || dict_table_is_comp(index->table));

  if (n_fields != 0) {
    n_null = index->has_instant_cols()
                 ? index->get_n_nullable_before(static_cast<uint32_t>(n_fields))
                 : index->n_nullable;
  }

  if (temp) {
    ut_ad(status == REC_STATUS_ORDINARY);
    ut_ad(n_fields <= dict_index_get_n_fields(index));
    n_node_ptr_field = ULINT_UNDEFINED;
    nulls = rec - 1;
    if (dict_table_is_comp(index->table)) {
      /* No need to do adjust fixed_len=0. We only
      need to adjust it for ROW_FORMAT=REDUNDANT. */
      temp = false;
    }
  } else {
    ut_ad(v_entry == NULL);
    ut_ad(num_v == 0);
    nulls = rec - (REC_N_NEW_EXTRA_BYTES + 1);

    switch (UNIV_EXPECT(status, REC_STATUS_ORDINARY)) {
      case REC_STATUS_ORDINARY:
        ut_ad(n_fields <= dict_index_get_n_fields(index));
        n_node_ptr_field = ULINT_UNDEFINED;

        if (index->has_instant_cols()) {
          uint32_t n_fields_len;
          n_fields_len = rec_set_n_fields(rec, n_fields);
          nulls -= n_fields_len;
          instant = true;
        }
        break;
      case REC_STATUS_NODE_PTR:
        ut_ad(n_fields ==
              static_cast<ulint>(
                  dict_index_get_n_unique_in_tree_nonleaf(index) + 1));
        n_node_ptr_field = n_fields - 1;
        n_null = index->n_instant_nullable;
        break;
      case REC_STATUS_INFIMUM:
      case REC_STATUS_SUPREMUM:
        ut_ad(n_fields == 1);
        n_node_ptr_field = ULINT_UNDEFINED;
        break;
      default:
        ut_error;
        return (instant);
    }
  }

  end = rec;

  if (n_fields != 0) {
    lens = nulls - UT_BITS_IN_BYTES(n_null);
    /* clear the SQL-null flags */
    memset(lens + 1, 0, nulls - lens);
  }

  /* Store the data and the offsets */

  for (i = 0; i < n_fields; i++) {
    const dict_field_t *ifield;
    dict_col_t *col = NULL;

    field = &fields[i];

    type = dfield_get_type(field);
    len = dfield_get_len(field);

    if (UNIV_UNLIKELY(i == n_node_ptr_field)) {
      ut_ad(dtype_get_prtype(type) & DATA_NOT_NULL);
      ut_ad(len == REC_NODE_PTR_SIZE);
      memcpy(end, dfield_get_data(field), len);
      end += REC_NODE_PTR_SIZE;
      break;
    }

    if (!(dtype_get_prtype(type) & DATA_NOT_NULL)) {
      /* nullable field */
      ut_ad(n_null--);

      if (UNIV_UNLIKELY(!(byte)null_mask)) {
        nulls--;
        null_mask = 1;
      }

      ut_ad(*nulls < null_mask);

      /* set the null flag if necessary */
      if (dfield_is_null(field)) {
        *nulls |= null_mask;
        null_mask <<= 1;
        continue;
      }

      null_mask <<= 1;
    }
    /* only nullable fields can be null */
    ut_ad(!dfield_is_null(field));

    ifield = index->get_field(i);
    fixed_len = ifield->fixed_len;
    col = ifield->col;
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
      ut_ad(!mbmaxlen || len >= mbminlen * (fixed_len / mbmaxlen));
      ut_ad(!dfield_is_ext(field));
#endif /* UNIV_DEBUG */
    } else if (dfield_is_ext(field)) {
      ut_ad(DATA_BIG_COL(col));
      ut_ad(len <= REC_ANTELOPE_MAX_INDEX_COL_LEN + BTR_EXTERN_FIELD_REF_SIZE);
      *lens-- = (byte)(len >> 8) | 0xc0;
      *lens-- = (byte)len;
    } else {
      /* DATA_POINT would have a fixed_len */
      ut_ad(dtype_get_mtype(type) != DATA_POINT);
#ifndef UNIV_HOTBACKUP
      ut_ad(len <= dtype_get_len(type) ||
            DATA_LARGE_MTYPE(dtype_get_mtype(type)) ||
            !strcmp(index->name, FTS_INDEX_TABLE_IND_NAME));
#endif /* !UNIV_HOTBACKUP */
      if (len < 128 ||
          !DATA_BIG_LEN_MTYPE(dtype_get_len(type), dtype_get_mtype(type))) {
        *lens-- = (byte)len;
      } else {
        ut_ad(len < 16384);
        *lens-- = (byte)(len >> 8) | 0x80;
        *lens-- = (byte)len;
      }
    }
    if (len > 0) memcpy(end, dfield_get_data(field), len);
    end += len;
  }

  if (!num_v) {
    return (instant);
  }

  /* reserve 2 bytes for writing length */
  byte *ptr = end;
  ptr += 2;

  /* Now log information on indexed virtual columns */
  for (ulint col_no = 0; col_no < num_v; col_no++) {
    dfield_t *vfield;
    ulint flen;

    const dict_v_col_t *col = dict_table_get_nth_v_col(index->table, col_no);

    if (col->m_col.ord_part || !dict_table_is_comp(index->table)) {
      ulint pos = col_no;

      pos += REC_MAX_N_FIELDS;

      ptr += mach_write_compressed(ptr, pos);

      vfield = dtuple_get_nth_v_field(v_entry, col->v_pos);

      flen = vfield->len;

      if (flen != UNIV_SQL_NULL) {
        /* The virtual column can only be in sec
        index, and index key length is bound by
        DICT_MAX_FIELD_LEN_BY_FORMAT */
        flen = std::min(flen, static_cast<ulint>(
                                DICT_MAX_FIELD_LEN_BY_FORMAT(index->table)));
      }

      ptr += mach_write_compressed(ptr, flen);

      if (flen != UNIV_SQL_NULL) {
        ut_memcpy(ptr, dfield_get_data(vfield), flen);
        ptr += flen;
      }
    }
  }

  mach_write_to_2(end, ptr - end);

  return (instant);
}
