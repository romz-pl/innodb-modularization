#include <innodb/page/page_is_comp.h>

#include <innodb/page/page_header_get_field.h>
#include <innodb/page/header.h>

/** Determine whether the page is in new-style compact format.
 @return nonzero if the page is in compact format, zero if it is in
 old-style format */
ulint page_is_comp(const page_t *page) /*!< in: index page */
{
  return (page_header_get_field(page, PAGE_N_HEAP) & 0x8000);
}
