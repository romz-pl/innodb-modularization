#include <innodb/page/page_dir_find_owner_slot.h>

#include <innodb/error/ut_error.h>
#include <innodb/logger/error.h>
#include <innodb/machine/data.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_dir_get_n_slots.h>
#include <innodb/page/page_dir_get_nth_slot.h>
#include <innodb/page/page_dir_slot_t.h>
#include <innodb/page/page_get_page_no.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_rec_check.h>
#include <innodb/record/rec_get_n_owned_new.h>
#include <innodb/record/rec_get_n_owned_old.h>
#include <innodb/record/rec_get_next_ptr_const.h>
#include <innodb/record/rec_print_old.h>

/** Looks for the directory slot which owns the given record.
 @return the directory slot number */
ulint page_dir_find_owner_slot(const rec_t *rec) /*!< in: the physical record */
{
  const page_t *page;
  uint16 rec_offs_bytes;
  const page_dir_slot_t *slot;
  const page_dir_slot_t *first_slot;
  const rec_t *r = rec;

  ut_ad(page_rec_check(rec));

  page = page_align(rec);
  first_slot = page_dir_get_nth_slot(page, 0);
  slot = page_dir_get_nth_slot(page, page_dir_get_n_slots(page) - 1);

  if (page_is_comp(page)) {
    while (rec_get_n_owned_new(r) == 0) {
      r = rec_get_next_ptr_const(r, TRUE);
      ut_ad(r >= page + PAGE_NEW_SUPREMUM);
      ut_ad(r < page + (UNIV_PAGE_SIZE - PAGE_DIR));
    }
  } else {
    while (rec_get_n_owned_old(r) == 0) {
      r = rec_get_next_ptr_const(r, FALSE);
      ut_ad(r >= page + PAGE_OLD_SUPREMUM);
      ut_ad(r < page + (UNIV_PAGE_SIZE - PAGE_DIR));
    }
  }

  rec_offs_bytes = mach_encode_2(r - page);

  while (UNIV_LIKELY(*(uint16 *)slot != rec_offs_bytes)) {
    if (UNIV_UNLIKELY(slot == first_slot)) {
      ib::error(ER_IB_MSG_860)
          << "Probable data corruption on page " << page_get_page_no(page)
          << ". Original record on that page;";

      if (page_is_comp(page)) {
        fputs("(compact record)", stderr);
      } else {
        rec_print_old(stderr, rec);
      }

      ib::error(ER_IB_MSG_861) << "Cannot find the dir slot for this"
                                  " record on that page;";

      if (page_is_comp(page)) {
        fputs("(compact record)", stderr);
      } else {
        rec_print_old(stderr, page + mach_decode_2(rec_offs_bytes));
      }

      ut_error;
    }

    slot += PAGE_DIR_SLOT_SIZE;
  }

  return (((ulint)(first_slot - slot)) / PAGE_DIR_SLOT_SIZE);
}
