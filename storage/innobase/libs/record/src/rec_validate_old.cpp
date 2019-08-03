#include <innodb/record/rec_validate_old.h>

#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_get_nth_field_offs_old.h>
#include <innodb/record/rec_get_nth_field_size.h>
#include <innodb/record/rec_get_data_size_old.h>
#include <innodb/record/flag.h>
#include <innodb/logger/error.h>

/** Validates the consistency of an old-style physical record.
 @return true if ok */
ibool rec_validate_old(const rec_t *rec) /*!< in: physical record */
{
  ulint len;
  ulint n_fields;
  ulint len_sum = 0;
  ulint i;

  ut_a(rec);
  n_fields = rec_get_n_fields_old_raw(rec);

  if ((n_fields == 0) || (n_fields > REC_MAX_N_FIELDS)) {
    ib::error(ER_IB_MSG_922) << "Record has " << n_fields << " fields";
    return (FALSE);
  }

  for (i = 0; i < n_fields; i++) {
    rec_get_nth_field_offs_old(rec, i, &len);

    if (!((len < UNIV_PAGE_SIZE) || (len == UNIV_SQL_NULL))) {
      ib::error(ER_IB_MSG_923) << "Record field " << i << " len " << len;
      return (FALSE);
    }

    if (len != UNIV_SQL_NULL) {
      len_sum += len;
    } else {
      len_sum += rec_get_nth_field_size(rec, i);
    }
  }

  if (len_sum != rec_get_data_size_old(rec)) {
    ib::error(ER_IB_MSG_924) << "Record len should be " << len_sum << ", len "
                             << rec_get_data_size_old(rec);
    return (FALSE);
  }

  return (TRUE);
}

