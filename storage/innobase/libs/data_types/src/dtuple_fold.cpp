#include <innodb/data_types/dtuple_fold.h>

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/random/random.h>


/** Compute a hash value of a prefix of an index record.
@param[in]	tuple		index record
@param[in]	n_fields	number of fields to include
@param[in]	n_bytes		number of bytes to fold in the last field
@param[in]	fold		fold value of the index identifier
@return the folded value */
ulint dtuple_fold(const dtuple_t *tuple, ulint n_fields, ulint n_bytes,
                  ulint fold) {
  const dfield_t *field;
  ulint i;
  const byte *data;
  ulint len;

  ut_ad(tuple);
  ut_ad(tuple->magic_n == DATA_TUPLE_MAGIC_N);
  ut_ad(dtuple_check_typed(tuple));

  for (i = 0; i < n_fields; i++) {
    field = dtuple_get_nth_field(tuple, i);

    data = (const byte *)dfield_get_data(field);
    len = dfield_get_len(field);

    if (len != UNIV_SQL_NULL) {
      fold = ut_fold_ulint_pair(fold, ut_fold_binary(data, len));
    }
  }

  if (n_bytes > 0) {
    field = dtuple_get_nth_field(tuple, i);

    data = (const byte *)dfield_get_data(field);
    len = dfield_get_len(field);

    if (len != UNIV_SQL_NULL) {
      if (len > n_bytes) {
        len = n_bytes;
      }

      fold = ut_fold_ulint_pair(fold, ut_fold_binary(data, len));
    }
  }

  return (fold);
}

