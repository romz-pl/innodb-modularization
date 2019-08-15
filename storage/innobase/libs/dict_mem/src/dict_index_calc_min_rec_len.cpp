#include <innodb/dict_mem/dict_index_calc_min_rec_len.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/bit/UT_BITS_IN_BYTES.h>

/** Calculates the minimum record length in an index. */
ulint dict_index_calc_min_rec_len(const dict_index_t *index) /*!< in: index */
{
  ulint sum = 0;
  ulint i;
  ulint comp = dict_table_is_comp(index->table);

  if (comp) {
    ulint nullable = 0;
    sum = REC_N_NEW_EXTRA_BYTES;
    for (i = 0; i < dict_index_get_n_fields(index); i++) {
      const dict_col_t *col = index->get_col(i);
      ulint size = col->get_fixed_size(comp);
      sum += size;
      if (!size) {
        size = col->len;
        sum += size < 128 ? 1 : 2;
      }
      if (!(col->prtype & DATA_NOT_NULL)) {
        nullable++;
      }
    }

    /* round the NULL flags up to full bytes */
    sum += UT_BITS_IN_BYTES(nullable);

    return (sum);
  }

  for (i = 0; i < dict_index_get_n_fields(index); i++) {
    sum += index->get_col(i)->get_fixed_size(comp);
  }

  if (sum > 127) {
    sum += 2 * dict_index_get_n_fields(index);
  } else {
    sum += dict_index_get_n_fields(index);
  }

  sum += REC_N_OLD_EXTRA_BYTES;

  return (sum);
}
