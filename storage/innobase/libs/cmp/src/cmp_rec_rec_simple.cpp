#include <innodb/cmp/cmp_rec_rec_simple.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/cmp/cmp_rec_rec_simple_field.h>
#include <innodb/cmp/cmp_rec_rec_simple_field.h>
#include <innodb/dict_mem/dict_index_get_n_unique.h>
#include <innodb/record/rec_offs_nth_sql_null.h>
#include <innodb/dict_mem/dict_index_is_unique.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>

void innobase_rec_to_mysql(struct TABLE *table, /*!< in/out: MySQL table */
                           const rec_t *rec,    /*!< in: record */
                           const dict_index_t *index, /*!< in: index */
                           const ulint *offsets) ;

/** Compare two physical records that contain the same number of columns,
none of which are stored externally.
@retval positive if rec1 (including non-ordering columns) is greater than rec2
@retval negative if rec1 (including non-ordering columns) is less than rec2
@retval 0 if rec1 is a duplicate of rec2 */
int cmp_rec_rec_simple(
    const rec_t *rec1,         /*!< in: physical record */
    const rec_t *rec2,         /*!< in: physical record */
    const ulint *offsets1,     /*!< in: rec_get_offsets(rec1, ...) */
    const ulint *offsets2,     /*!< in: rec_get_offsets(rec2, ...) */
    const dict_index_t *index, /*!< in: data dictionary index */
    struct TABLE *table)       /*!< in: MySQL table, for reporting
                               duplicate key value if applicable,
                               or NULL */
{
  ulint n;
  ulint n_uniq = dict_index_get_n_unique(index);
  bool null_eq = false;

  ut_ad(rec_offs_n_fields(offsets1) >= n_uniq);
  ut_ad(rec_offs_n_fields(offsets2) == rec_offs_n_fields(offsets2));

  ut_ad(rec_offs_comp(offsets1) == rec_offs_comp(offsets2));

  for (n = 0; n < n_uniq; n++) {
    int cmp =
        cmp_rec_rec_simple_field(rec1, rec2, offsets1, offsets2, index, n);

    if (cmp) {
      return (cmp);
    }

    /* If the fields are internally equal, they must both
    be NULL or non-NULL. */
    ut_ad(rec_offs_nth_sql_null(offsets1, n) ==
          rec_offs_nth_sql_null(offsets2, n));

    if (rec_offs_nth_sql_null(offsets1, n)) {
      ut_ad(!(index->get_col(n)->prtype & DATA_NOT_NULL));
      null_eq = true;
    }
  }

  /* If we ran out of fields, the ordering columns of rec1 were
  equal to rec2. Issue a duplicate key error if needed. */

  if (!null_eq && table && dict_index_is_unique(index)) {
    /* Report erroneous row using new version of table. */
    innobase_rec_to_mysql(table, rec1, index, offsets1);
    return (0);
  }

  /* Else, keep comparing so that we have the full internal
  order. */
  for (; n < dict_index_get_n_fields(index); n++) {
    int cmp =
        cmp_rec_rec_simple_field(rec1, rec2, offsets1, offsets2, index, n);

    if (cmp) {
      return (cmp);
    }

    /* If the fields are internally equal, they must both
    be NULL or non-NULL. */
    ut_ad(rec_offs_nth_sql_null(offsets1, n) ==
          rec_offs_nth_sql_null(offsets2, n));
  }

  /* This should never be reached. Internally, an index must
  never contain duplicate entries. */
  ut_ad(0);
  return (0);
}
