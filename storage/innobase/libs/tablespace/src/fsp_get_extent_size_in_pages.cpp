#include <innodb/tablespace/fsp_get_extent_size_in_pages.h>

#include <innodb/tablespace/consts.h>

/** Calculate the number of physical pages in an extent for this file.
@param[in]	page_size	page_size of the datafile
@return number of pages in an extent for this file. */
page_no_t fsp_get_extent_size_in_pages(const page_size_t &page_size) {
  return (static_cast<page_no_t>(FSP_EXTENT_SIZE * UNIV_PAGE_SIZE /
                                 page_size.physical()));
}
