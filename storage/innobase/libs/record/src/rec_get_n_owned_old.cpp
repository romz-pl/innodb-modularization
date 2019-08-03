#include <innodb/record/rec_get_n_owned_old.h>

#include <innodb/record/rec_get_bit_field_1.h>
#include <innodb/record/flag.h>

/** The following function is used to get the number of records owned by the
 previous directory record.
 @return number of owned records */
ulint rec_get_n_owned_old(
    const rec_t *rec) /*!< in: old-style physical record */
{
  return (rec_get_bit_field_1(rec, REC_OLD_N_OWNED, REC_N_OWNED_MASK,
                              REC_N_OWNED_SHIFT));
}
