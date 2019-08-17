#include <innodb/page/page_mem_alloc_free.h>

#include <innodb/assert/assert.h>
#include <innodb/page/header.h>
#include <innodb/page/page_header_get_field.h>
#include <innodb/page/page_header_get_ptr.h>
#include <innodb/page/page_header_set_field.h>
#include <innodb/page/page_header_set_ptr.h>
#include <innodb/record/rec_get_next_offs.h>

/** Allocates a block of memory from the free list of an index page. */
void page_mem_alloc_free(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page with enough
                             space available for inserting the record,
                             or NULL */
    rec_t *next_rec,          /*!< in: pointer to the new head of the
                             free record list */
    ulint need)               /*!< in: number of bytes allocated */
{
  ulint garbage;

#ifdef UNIV_DEBUG
  const rec_t *old_rec = page_header_get_ptr(page, PAGE_FREE);
  ulint next_offs;

  ut_ad(old_rec);
  next_offs = rec_get_next_offs(old_rec, page_is_comp(page));
  ut_ad(next_rec == (next_offs ? page + next_offs : NULL));
#endif

  page_header_set_ptr(page, page_zip, PAGE_FREE, next_rec);

  garbage = page_header_get_field(page, PAGE_GARBAGE);
  ut_ad(garbage >= need);

  page_header_set_field(page, page_zip, PAGE_GARBAGE, garbage - need);
}
