#include <innodb/record/rec_validate.h>

#include <innodb/logger/error.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_get_nth_field_offs.h>
#include <innodb/record/rec_get_nth_field_size.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_validate_old.h>

/** Validates the consistency of a physical record.
 @return true if ok */
ibool rec_validate(
    const rec_t *rec,     /*!< in: physical record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ulint len;
  uint16_t n_fields;
  ulint len_sum = 0;
  ulint i;
  uint16_t n_defaults = 0;

  ut_a(rec);
  n_fields = static_cast<uint16_t>(rec_offs_n_fields(offsets));

  if ((n_fields == 0) || (n_fields > REC_MAX_N_FIELDS)) {
    ib::error(ER_IB_MSG_925) << "Record has " << n_fields << " fields";
    return (FALSE);
  }

  for (i = 0; i < n_fields; i++) {
    rec_get_nth_field_offs(offsets, i, &len);

    switch (len) {
      case UNIV_SQL_ADD_COL_DEFAULT:
        ++n_defaults;
        break;
      case UNIV_SQL_NULL:
        if (!rec_offs_comp(offsets)) {
          /* If a default value is NULL, it will come
          here, however, this is not inlined, so
          don't count it */
          if (i < rec_get_n_fields_old_raw(rec)) {
            len_sum += rec_get_nth_field_size(rec, i);
          } else {
            ++n_defaults;
          }
        }
        break;
      default:
        ut_a(n_defaults == 0);
        if (len >= UNIV_PAGE_SIZE) {
          ib::error(ER_IB_MSG_926) << "Record field " << i << " len " << len;
          return (FALSE);
        }

        len_sum += len;
        break;
    }
  }

  ut_a(rec_offs_comp(offsets) ||
       n_fields <= rec_get_n_fields_old_raw(rec) + n_defaults);

  if (len_sum != rec_offs_data_size(offsets)) {
    ib::error(ER_IB_MSG_927) << "Record len should be " << len_sum << ", len "
                             << rec_offs_data_size(offsets);
    return (FALSE);
  }

  if (!rec_offs_comp(offsets)) {
    ut_a(rec_validate_old(rec));
  }

  return (TRUE);
}


