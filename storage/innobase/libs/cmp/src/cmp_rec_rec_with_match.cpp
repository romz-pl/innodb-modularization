#include <innodb/cmp/cmp_rec_rec_with_match.h>

#include <innodb/cmp/cmp_data.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree.h>
#include <innodb/dict_mem/dict_index_is_ibuf.h>
#include <innodb/dict_mem/dict_index_is_spatial.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_get_nth_field_instant.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_offs_n_fields.h>


/** Compare two B-tree records.
@param[in] rec1 B-tree record
@param[in] rec2 B-tree record
@param[in] offsets1 rec_get_offsets(rec1, index)
@param[in] offsets2 rec_get_offsets(rec2, index)
@param[in] index B-tree index
@param[in] nulls_unequal true if this is for index cardinality
statistics estimation, and innodb_stats_method=nulls_unequal
or innodb_stats_method=nulls_ignored
@param[out] matched_fields number of completely matched fields
within the first field not completely matched
@return the comparison result
@retval 0 if rec1 is equal to rec2
@retval negative if rec1 is less than rec2
@retval positive if rec2 is greater than rec2 */
int cmp_rec_rec_with_match(const rec_t *rec1, const rec_t *rec2,
                           const ulint *offsets1, const ulint *offsets2,
                           const dict_index_t *index, bool nulls_unequal,
                           ulint *matched_fields) {
  ulint rec1_n_fields;    /* the number of fields in rec */
  ulint rec1_f_len;       /* length of current field in rec */
  const byte *rec1_b_ptr; /* pointer to the current byte
                          in rec field */
  ulint rec2_n_fields;    /* the number of fields in rec */
  ulint rec2_f_len;       /* length of current field in rec */
  const byte *rec2_b_ptr; /* pointer to the current byte
                          in rec field */
  ulint cur_field = 0;    /* current field number */
  int ret = 0;            /* return value */
  ulint comp;

  ut_ad(rec1 != NULL);
  ut_ad(rec2 != NULL);
  ut_ad(index != NULL);
  ut_ad(rec_offs_validate(rec1, index, offsets1));
  ut_ad(rec_offs_validate(rec2, index, offsets2));
  ut_ad(rec_offs_comp(offsets1) == rec_offs_comp(offsets2));

  comp = rec_offs_comp(offsets1);
  rec1_n_fields = rec_offs_n_fields(offsets1);
  rec2_n_fields = rec_offs_n_fields(offsets2);

  /* Test if rec is the predefined minimum record */
  if (UNIV_UNLIKELY(rec_get_info_bits(rec1, comp) & REC_INFO_MIN_REC_FLAG)) {
    /* There should only be one such record. */
    ut_ad(!(rec_get_info_bits(rec2, comp) & REC_INFO_MIN_REC_FLAG));
    ret = -1;
    goto order_resolved;
  } else if (UNIV_UNLIKELY(rec_get_info_bits(rec2, comp) &
                           REC_INFO_MIN_REC_FLAG)) {
    ret = 1;
    goto order_resolved;
  }

  /* Match fields in a loop */

  for (; cur_field < rec1_n_fields && cur_field < rec2_n_fields; cur_field++) {
    ulint mtype;
    ulint prtype;
    bool is_asc;

    /* If this is node-ptr records then avoid comparing node-ptr
    field. Only key field needs to be compared. */
    if (cur_field == dict_index_get_n_unique_in_tree(index)) {
      break;
    }

    if (dict_index_is_ibuf(index)) {
      /* This is for the insert buffer B-tree. */
      mtype = DATA_BINARY;
      prtype = 0;
      is_asc = true;
    } else {
      const dict_col_t *col;
      const dict_field_t *field = index->get_field(cur_field);

      col = index->get_col(cur_field);

      mtype = col->mtype;
      prtype = col->prtype;
      is_asc = field->is_ascending;

      /* If the index is spatial index, we mark the
      prtype of the first field as MBR field. */
      if (cur_field == 0 && dict_index_is_spatial(index)) {
        ut_ad(DATA_GEOMETRY_MTYPE(mtype));
        prtype |= DATA_GIS_MBR;
      }
    }

    /* We should never encounter an externally stored field.
    Externally stored fields only exist in clustered index
    leaf page records. These fields should already differ
    in the primary key columns already, before DB_TRX_ID,
    DB_ROLL_PTR, and any externally stored columns. */
    ut_ad(!rec_offs_nth_extern(offsets1, cur_field));
    ut_ad(!rec_offs_nth_extern(offsets2, cur_field));

    rec1_b_ptr = rec_get_nth_field_instant(rec1, offsets1, cur_field, index,
                                           &rec1_f_len);
    rec2_b_ptr = rec_get_nth_field_instant(rec2, offsets2, cur_field, index,
                                           &rec2_f_len);

    if (nulls_unequal && rec1_f_len == UNIV_SQL_NULL &&
        rec2_f_len == UNIV_SQL_NULL) {
      ret = -1;
      goto order_resolved;
    }

    ret = cmp_data(mtype, prtype, is_asc, rec1_b_ptr, rec1_f_len, rec2_b_ptr,
                   rec2_f_len);
    if (ret) {
      goto order_resolved;
    }
  }

  /* If we ran out of fields, rec1 was equal to rec2 up
  to the common fields */
  ut_ad(ret == 0);
order_resolved:
  *matched_fields = cur_field;
  return (ret);
}
