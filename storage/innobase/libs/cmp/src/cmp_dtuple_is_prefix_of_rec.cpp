#include <innodb/cmp/cmp_dtuple_is_prefix_of_rec.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_is_spatial.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/cmp/cmp_dtuple_rec_with_match.h>


/** Checks if a dtuple is a prefix of a record. The last field in dtuple is
allowed to be a prefix of the corresponding field in the record.
@param[in]	dtuple	data tuple
@param[in]	rec	B-tree record
@param[in]	index	B-tree index
@param[in]	offsets	rec_get_offsets(rec)
@return true if prefix */
ibool cmp_dtuple_is_prefix_of_rec(const dtuple_t *dtuple, const rec_t *rec,
                                  const dict_index_t *index,
                                  const ulint *offsets) {
  ut_ad(!dict_index_is_spatial(index));

  ulint n_fields;
  ulint matched_fields = 0;

  n_fields = dtuple_get_n_fields(dtuple);

  if (n_fields > rec_offs_n_fields(offsets)) {
    ut_ad(0);
    return (FALSE);
  }

  return (
      !cmp_dtuple_rec_with_match(dtuple, rec, index, offsets, &matched_fields));
}
