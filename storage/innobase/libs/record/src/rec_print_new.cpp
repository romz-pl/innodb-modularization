#include <innodb/record/rec_print_new.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_print_old.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_print_comp.h>
#include <innodb/record/rec_validate.h>
#include <innodb/assert/assert.h>

/** Prints a physical record. */
void rec_print_new(
    FILE *file,           /*!< in: file where to print */
    const rec_t *rec,     /*!< in: physical record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ut_ad(rec);
  ut_ad(offsets);
  ut_ad(rec_offs_validate(rec, NULL, offsets));

#ifdef UNIV_DEBUG
  if (rec_get_deleted_flag(rec, rec_offs_comp(offsets))) {
    DBUG_PRINT("info", ("deleted "));
  } else {
    DBUG_PRINT("info", ("not-deleted "));
  }
#endif /* UNIV_DEBUG */

  if (!rec_offs_comp(offsets)) {
    rec_print_old(file, rec);
    return;
  }

  fprintf(file,
          "PHYSICAL RECORD: n_fields %lu;"
          " compact format; info bits %lu\n",
          (ulong)rec_offs_n_fields(offsets),
          (ulong)rec_get_info_bits(rec, TRUE));

  rec_print_comp(file, rec, offsets);
  rec_validate(rec, offsets);
}


#endif
