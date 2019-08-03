#include <innodb/record/rec_set_n_owned_old.h>

#include <innodb/record/rec_set_bit_field_1.h>
#include <innodb/record/flag.h>

/** The following function is used to set the number of owned records. */
void rec_set_n_owned_old(rec_t *rec,    /*!< in: old-style physical record */
                         ulint n_owned) /*!< in: the number of owned */
{
  rec_set_bit_field_1(rec, n_owned, REC_OLD_N_OWNED, REC_N_OWNED_MASK,
                      REC_N_OWNED_SHIFT);
}
