#include <innodb/cmp/cmp_rec_rec_simple_field.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/record/rec_get_nth_field_instant.h>
#include <innodb/cmp/cmp_data.h>


/** Compare two physical record fields.
 @retval positive if rec1 field is greater than rec2
 @retval negative if rec1 field is less than rec2
 @retval 0 if rec1 field equals to rec2 */
MY_ATTRIBUTE((warn_unused_result)) int cmp_rec_rec_simple_field(
    const rec_t *rec1,         /*!< in: physical record */
    const rec_t *rec2,         /*!< in: physical record */
    const ulint *offsets1,     /*!< in: rec_get_offsets(rec1, ...) */
    const ulint *offsets2,     /*!< in: rec_get_offsets(rec2, ...) */
    const dict_index_t *index, /*!< in: data dictionary index */
    ulint n)                   /*!< in: field to compare */
{
  const byte *rec1_b_ptr;
  const byte *rec2_b_ptr;
  ulint rec1_f_len;
  ulint rec2_f_len;
  const dict_col_t *col = index->get_col(n);
  const dict_field_t *field = index->get_field(n);

  ut_ad(!rec_offs_nth_extern(offsets1, n));
  ut_ad(!rec_offs_nth_extern(offsets2, n));

  rec1_b_ptr = rec_get_nth_field_instant(rec1, offsets1, n, index, &rec1_f_len);
  rec2_b_ptr = rec_get_nth_field_instant(rec2, offsets2, n, index, &rec2_f_len);

  return (cmp_data(col->mtype, col->prtype, field->is_ascending, rec1_b_ptr,
                   rec1_f_len, rec2_b_ptr, rec2_f_len));
}
