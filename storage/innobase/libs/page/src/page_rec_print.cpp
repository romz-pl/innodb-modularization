#include <innodb/page/page_rec_print.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/logger/info.h>
#include <innodb/page/page_rec_check.h>
#include <innodb/page/page_rec_is_comp.h>
#include <innodb/record/rec_get_heap_no_new.h>
#include <innodb/record/rec_get_heap_no_old.h>
#include <innodb/record/rec_get_n_owned_new.h>
#include <innodb/record/rec_get_n_owned_old.h>
#include <innodb/record/rec_get_next_offs.h>
#include <innodb/record/rec_get_next_offs.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_print_new.h>
#include <innodb/record/rec_validate.h>


/** Prints record contents including the data relevant only in
 the index page context. */
void page_rec_print(const rec_t *rec,     /*!< in: physical record */
                    const ulint *offsets) /*!< in: record descriptor */
{
  ut_a(!page_rec_is_comp(rec) == !rec_offs_comp(offsets));
  rec_print_new(stderr, rec, offsets);
  if (page_rec_is_comp(rec)) {
    ib::info(ER_IB_MSG_863) << "n_owned: " << rec_get_n_owned_new(rec)
                            << "; heap_no: " << rec_get_heap_no_new(rec)
                            << "; next rec: " << rec_get_next_offs(rec, TRUE);
  } else {
    ib::info(ER_IB_MSG_864) << "n_owned: " << rec_get_n_owned_old(rec)
                            << "; heap_no: " << rec_get_heap_no_old(rec)
                            << "; next rec: " << rec_get_next_offs(rec, FALSE);
  }

  page_rec_check(rec);
  rec_validate(rec, offsets);
}

#endif

