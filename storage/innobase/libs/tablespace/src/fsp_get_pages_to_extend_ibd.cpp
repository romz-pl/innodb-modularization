#include <innodb/tablespace/fsp_get_pages_to_extend_ibd.h>

#include <innodb/page/page_size_t.h>
#include <innodb/tablespace/fsp_get_extent_size_in_pages.h>
#include <innodb/tablespace/consts.h>

/** Calculate the number of pages to extend a datafile.
We extend single-table and general tablespaces first one extent at a time,
but 4 at a time for bigger tablespaces. It is not enough to extend always
by one extent, because we need to add at least one extent to FSP_FREE.
A single extent descriptor page will track many extents. And the extent
that uses its extent descriptor page is put onto the FSP_FREE_FRAG list.
Extents that do not use their extent descriptor page are added to FSP_FREE.
The physical page size is used to determine how many extents are tracked
on one extent descriptor page. See xdes_calc_descriptor_page().
@param[in]	page_size	page_size of the datafile
@param[in]	size		current number of pages in the datafile
@return number of pages to extend the file. */
page_no_t fsp_get_pages_to_extend_ibd(const page_size_t &page_size,
                                      page_no_t size) {
  page_no_t size_increase; /* number of pages to extend this file */
  page_no_t extent_size;   /* one megabyte, in pages */
  page_no_t threshold;     /* The size of the tablespace (in number
                           of pages) where we start allocating more
                           than one extent at a time. */

  extent_size = fsp_get_extent_size_in_pages(page_size);

  /* The threshold is set at 32MiB except when the physical page
  size is small enough that it must be done sooner. */
  threshold =
      std::min(32 * extent_size, static_cast<page_no_t>(page_size.physical()));

  if (size < threshold) {
    size_increase = extent_size;
  } else {
    /* Below in fsp_fill_free_list() we assume
    that we add at most FSP_FREE_ADD extents at
    a time */
    size_increase = FSP_FREE_ADD * extent_size;
  }

  return (size_increase);
}

