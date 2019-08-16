#include <innodb/cmp/cmp_dtuple_rec_with_match_low.h>

#include <innodb/cmp/cmp_data.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_get_info_bits.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/dict_mem/dict_index_is_ibuf.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_validate.h>


/** Compare a data tuple to a physical record.
@param[in]	dtuple		data tuple
@param[in]	rec		record
@param[in]	index		index
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	n_cmp		number of fields to compare
@param[in,out]	matched_fields	number of completely matched fields
@return the comparison result of dtuple and rec
@retval 0 if dtuple is equal to rec
@retval negative if dtuple is less than rec
@retval positive if dtuple is greater than rec */
int cmp_dtuple_rec_with_match_low(const dtuple_t *dtuple, const rec_t *rec,
                                  const dict_index_t *index,
                                  const ulint *offsets, ulint n_cmp,
                                  ulint *matched_fields) {
  ulint cur_field; /* current field number */
  int ret;         /* return value */

  ut_ad(dtuple_check_typed(dtuple));
  ut_ad(rec_offs_validate(rec, index, offsets));

  cur_field = *matched_fields;

  ut_ad(n_cmp > 0);
  ut_ad(n_cmp <= dtuple_get_n_fields(dtuple));
  ut_ad(cur_field <= n_cmp);
  ut_ad(cur_field <= rec_offs_n_fields(offsets));

  if (cur_field == 0) {
    ulint rec_info = rec_get_info_bits(rec, rec_offs_comp(offsets));
    ulint tup_info = dtuple_get_info_bits(dtuple);

    /* The leftmost node pointer record is defined as
    smaller than any other node pointer, independent of
    any ASC/DESC flags. It is an "infimum node pointer". */
    if (UNIV_UNLIKELY(rec_info & REC_INFO_MIN_REC_FLAG)) {
      ret = !(tup_info & REC_INFO_MIN_REC_FLAG);
      goto order_resolved;
    } else if (UNIV_UNLIKELY(tup_info & REC_INFO_MIN_REC_FLAG)) {
      ret = -1;
      goto order_resolved;
    }
  }

  /* Match fields in a loop */

  for (; cur_field < n_cmp; cur_field++) {
    const byte *rec_b_ptr;
    const dfield_t *dtuple_field = dtuple_get_nth_field(dtuple, cur_field);
    const byte *dtuple_b_ptr =
        static_cast<const byte *>(dfield_get_data(dtuple_field));
    const dtype_t *type = dfield_get_type(dtuple_field);
    ulint dtuple_f_len = dfield_get_len(dtuple_field);
    ulint rec_f_len;

    /* We should never compare against an externally
    stored field.  Only clustered index records can
    contain externally stored fields, and the first fields
    (primary key fields) should already differ. */
    ut_ad(!rec_offs_nth_extern(offsets, cur_field));
    /* So does the field with default value */
    ut_ad(!rec_offs_nth_default(offsets, cur_field));

    rec_b_ptr = rec_get_nth_field(rec, offsets, cur_field, &rec_f_len);

    ut_ad(!dfield_is_ext(dtuple_field));

    /* For now, change buffering is only supported on
    indexes with ascending order on the columns. */
    ret = cmp_data(
        type->mtype, type->prtype,
        dict_index_is_ibuf(index) || index->get_field(cur_field)->is_ascending,
        dtuple_b_ptr, dtuple_f_len, rec_b_ptr, rec_f_len);
    if (ret) {
      goto order_resolved;
    }
  }

  ret = 0; /* If we ran out of fields, dtuple was equal to rec
           up to the common fields */
order_resolved:
  *matched_fields = cur_field;
  return (ret);
}

