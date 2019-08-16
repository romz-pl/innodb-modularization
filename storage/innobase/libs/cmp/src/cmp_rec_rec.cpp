#include <innodb/cmp/cmp_rec_rec.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/cmp/cmp_rec_rec_with_match.h>


/** Compare two B-tree records.
Only the common first fields are compared, and externally stored field
are treated as equal.
@param[in]	rec1		B-tree record
@param[in]	rec2		B-tree record
@param[in]	offsets1	rec_get_offsets(rec1, index)
@param[in]	offsets2	rec_get_offsets(rec2, index)
@param[out]	matched_fields	number of completely matched fields
                                within the first field not completely matched
@return positive, 0, negative if rec1 is greater, equal, less, than rec2,
respectively */
int cmp_rec_rec(const rec_t *rec1, const rec_t *rec2, const ulint *offsets1,
                const ulint *offsets2, const dict_index_t *index,
                ulint *matched_fields) {
  ulint match_f;
  int ret;

  ret = cmp_rec_rec_with_match(rec1, rec2, offsets1, offsets2, index, false,
                               &match_f);

  if (matched_fields != NULL) {
    *matched_fields = match_f;
  }

  return (ret);
}
