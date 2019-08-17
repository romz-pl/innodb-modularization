#include <innodb/page/page_rec_validate.h>

#include <innodb/logger/warn.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_rec_is_comp.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_heap_no_old.h>
#include <innodb/record/rec_get_n_owned_new.h>
#include <innodb/record/rec_get_n_owned_old.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_validate.h>


/** The following is used to validate a record on a page. This function
 differs from rec_validate as it can also check the n_owned field and
 the heap_no field.
 @return true if ok */
ibool page_rec_validate(
    const rec_t *rec,     /*!< in: physical record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ulint n_owned;
  ulint heap_no;
  const page_t *page;

  page = page_align(rec);
  ut_a(!page_is_comp(page) == !rec_offs_comp(offsets));

  page_rec_check(rec);
  rec_validate(rec, offsets);

  if (page_rec_is_comp(rec)) {
    n_owned = rec_get_n_owned_new(rec);
    heap_no = rec_get_heap_no_new(rec);
  } else {
    n_owned = rec_get_n_owned_old(rec);
    heap_no = rec_get_heap_no_old(rec);
  }

  if (UNIV_UNLIKELY(!(n_owned <= PAGE_DIR_SLOT_MAX_N_OWNED))) {
    ib::warn(ER_IB_MSG_865) << "Dir slot of rec " << page_offset(rec)
                            << ", n owned too big " << n_owned;
    return (FALSE);
  }

  if (UNIV_UNLIKELY(!(heap_no < page_dir_get_n_heap(page)))) {
    ib::warn(ER_IB_MSG_866)
        << "Heap no of rec " << page_offset(rec) << " too big " << heap_no
        << " " << page_dir_get_n_heap(page);
    return (FALSE);
  }

  return (TRUE);
}
