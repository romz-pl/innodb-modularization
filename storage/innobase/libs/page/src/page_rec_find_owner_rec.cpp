#include <innodb/page/page_rec_find_owner_rec.h>

#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_rec_is_comp.h>
#include <innodb/page/page_rec_get_next.h>
#include <innodb/record/rec_get_n_owned_new.h>
#include <innodb/record/rec_get_n_owned_old.h>
#include <innodb/assert/assert.h>

/** Looks for the record which owns the given record.
 @return the owner record */
rec_t *page_rec_find_owner_rec(rec_t *rec) /*!< in: the physical record */
{
  ut_ad(page_rec_check(rec));

  if (page_rec_is_comp(rec)) {
    while (rec_get_n_owned_new(rec) == 0) {
      rec = page_rec_get_next(rec);
    }
  } else {
    while (rec_get_n_owned_old(rec) == 0) {
      rec = page_rec_get_next(rec);
    }
  }

  return (rec);
}
