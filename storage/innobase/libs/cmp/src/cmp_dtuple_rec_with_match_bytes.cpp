#include <innodb/cmp/cmp_dtuple_rec_with_match_bytes.h>

#include <innodb/cmp/cmp_data.h>
#include <innodb/cmp/cmp_get_pad_char.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_get_n_fields_cmp.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/dict_mem/dict_index_is_ibuf.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_get_nth_field.h>


/** Compare a data tuple to a physical record.
@param[in]	dtuple		data tuple
@param[in]	rec		B-tree or R-tree index record
@param[in]	index		index tree
@param[in]	offsets		rec_get_offsets(rec)
@param[in,out]	matched_fields	number of completely matched fields
@param[in,out]	matched_bytes	number of matched bytes in the first
field that is not matched
@return the comparison result of dtuple and rec
@retval 0 if dtuple is equal to rec
@retval negative if dtuple is less than rec
@retval positive if dtuple is greater than rec */
int cmp_dtuple_rec_with_match_bytes(const dtuple_t *dtuple, const rec_t *rec,
                                    const dict_index_t *index,
                                    const ulint *offsets, ulint *matched_fields,
                                    ulint *matched_bytes) {
  ulint n_cmp = dtuple_get_n_fields_cmp(dtuple);
  ulint cur_field; /* current field number */
  ulint cur_bytes;
  int ret; /* return value */

  ut_ad(dtuple_check_typed(dtuple));
  ut_ad(rec_offs_validate(rec, index, offsets));
  // ut_ad(page_is_leaf(page_align(rec)));
  ut_ad(!(REC_INFO_MIN_REC_FLAG & dtuple_get_info_bits(dtuple)));
  ut_ad(!(REC_INFO_MIN_REC_FLAG &
          rec_get_info_bits(rec, rec_offs_comp(offsets))));

  cur_field = *matched_fields;
  cur_bytes = *matched_bytes;

  ut_ad(n_cmp <= dtuple_get_n_fields(dtuple));
  ut_ad(cur_field <= n_cmp);
  ut_ad(cur_field + (cur_bytes > 0) <= rec_offs_n_fields(offsets));

  /* Match fields in a loop; stop if we run out of fields in dtuple
  or find an externally stored field */

  while (cur_field < n_cmp) {
    const dfield_t *dfield = dtuple_get_nth_field(dtuple, cur_field);
    const dtype_t *type = dfield_get_type(dfield);
    ulint dtuple_f_len = dfield_get_len(dfield);
    const byte *dtuple_b_ptr;
    const byte *rec_b_ptr;
    ulint rec_f_len;

    ut_ad(!rec_offs_nth_default(offsets, cur_field));

    /* For now, change buffering is only supported on
    indexes with ascending order on the columns. */
    const bool is_ascending =
        dict_index_is_ibuf(index) || index->fields[cur_field].is_ascending;

    dtuple_b_ptr = static_cast<const byte *>(dfield_get_data(dfield));
    rec_b_ptr = rec_get_nth_field(rec, offsets, cur_field, &rec_f_len);
    ut_ad(!rec_offs_nth_extern(offsets, cur_field));

    /* If we have matched yet 0 bytes, it may be that one or
    both the fields are SQL null, or the record or dtuple may be
    the predefined minimum record. */
    if (cur_bytes == 0) {
      if (dtuple_f_len == UNIV_SQL_NULL) {
        if (rec_f_len == UNIV_SQL_NULL) {
          goto next_field;
        }

        ret = is_ascending ? -1 : 1;
        goto order_resolved;
      } else if (rec_f_len == UNIV_SQL_NULL) {
        /* We define the SQL null to be the
        smallest possible value of a field
        in the alphabetical order */

        ret = is_ascending ? 1 : -1;
        goto order_resolved;
      }
    }

    switch (type->mtype) {
      case DATA_FIXBINARY:
      case DATA_BINARY:
      case DATA_INT:
      case DATA_SYS_CHILD:
      case DATA_SYS:
        break;
      case DATA_BLOB:
        if (type->prtype & DATA_BINARY_TYPE) {
          break;
        }
        /* fall through */
      default:
        ret = cmp_data(type->mtype, type->prtype, is_ascending, dtuple_b_ptr,
                       dtuple_f_len, rec_b_ptr, rec_f_len);

        if (!ret) {
          goto next_field;
        }

        cur_bytes = 0;
        goto order_resolved;
    }

    /* Set the pointers at the current byte */

    rec_b_ptr += cur_bytes;
    dtuple_b_ptr += cur_bytes;
    /* Compare then the fields */

    for (const ulint pad = cmp_get_pad_char(type);; cur_bytes++) {
      ulint rec_byte = pad;
      ulint dtuple_byte = pad;

      if (rec_f_len <= cur_bytes) {
        if (dtuple_f_len <= cur_bytes) {
          goto next_field;
        }

        if (rec_byte == ULINT_UNDEFINED) {
          ret = is_ascending ? 1 : -1;
          goto order_resolved;
        }
      } else {
        rec_byte = *rec_b_ptr++;
      }

      if (dtuple_f_len <= cur_bytes) {
        if (dtuple_byte == ULINT_UNDEFINED) {
          ret = is_ascending ? -1 : 1;
          goto order_resolved;
        }
      } else {
        dtuple_byte = *dtuple_b_ptr++;
      }

      if (dtuple_byte < rec_byte) {
        ret = is_ascending ? -1 : 1;
        goto order_resolved;
      } else if (dtuple_byte > rec_byte) {
        ret = is_ascending ? 1 : -1;
        goto order_resolved;
      }
    }

  next_field:
    cur_field++;
    cur_bytes = 0;
  }

  ut_ad(cur_bytes == 0);

  ret = 0; /* If we ran out of fields, dtuple was equal to rec
           up to the common fields */
order_resolved:
  *matched_fields = cur_field;
  *matched_bytes = cur_bytes;

  return (ret);
}
