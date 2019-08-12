#include <innodb/record/rec_set_n_owned_new.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/record/rec_set_bit_field_1.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/page/page_zip_rec_set_owned.h>
#include <innodb/record/flag.h>


/** The following function is used to set the number of owned records. */
void rec_set_n_owned_new(
    rec_t *rec,               /*!< in/out: new-style physical record */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    ulint n_owned)            /*!< in: the number of owned */
{
  rec_set_bit_field_1(rec, n_owned, REC_NEW_N_OWNED, REC_N_OWNED_MASK,
                      REC_N_OWNED_SHIFT);
  if (page_zip && rec_get_status(rec) != REC_STATUS_SUPREMUM) {
    page_zip_rec_set_owned(page_zip, rec, n_owned);
  }
}
