#include <innodb/page/page_rec_get_next_low.h>

#include <innodb/assert/assert.h>
#include <innodb/error/ut_error.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_get_page_no.h>
#include <innodb/page/page_get_space_id.h>
#include <innodb/page/page_offset.h>
#include <innodb/record/rec_get_next_offs.h>

#include <cstdio>

/** Gets the pointer to the next record on the page.
 @return pointer to next record */
const rec_t *page_rec_get_next_low(
    const rec_t *rec, /*!< in: pointer to record */
    ulint comp)       /*!< in: nonzero=compact page layout */
{
  ulint offs;
  const page_t *page;

  ut_ad(page_rec_check(rec));

  page = page_align(rec);

  offs = rec_get_next_offs(rec, comp);

  if (offs >= UNIV_PAGE_SIZE) {
    fprintf(stderr,
            "InnoDB: Next record offset is nonsensical %lu"
            " in record at offset %lu\n"
            "InnoDB: rec address %p, space id %lu, page %lu\n",
            (ulong)offs, (ulong)page_offset(rec), (void *)rec,
            (ulong)page_get_space_id(page), (ulong)page_get_page_no(page));
    ut_error;
  } else if (offs == 0) {
    return (NULL);
  }

  return (page + offs);
}
