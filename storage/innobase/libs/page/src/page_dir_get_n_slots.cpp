#include <innodb/page/page_dir_get_n_slots.h>

#include <innodb/page/page_header_get_field.h>
#include <innodb/page/header.h>

/** Gets the number of dir slots in directory.
 @return number of slots */
ulint page_dir_get_n_slots(const page_t *page) /*!< in: index page */
{
  return (page_header_get_field(page, PAGE_N_DIR_SLOTS));
}
