#include <innodb/page/page_dir_set_n_slots.h>

#include <innodb/page/page_header_set_field.h>

#include <innodb/page/header.h>

/** Sets the number of dir slots in directory. */
void page_dir_set_n_slots(
    page_t *page,             /*!< in/out: page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be updated, or NULL */
    ulint n_slots)            /*!< in: number of slots */
{
  page_header_set_field(page, page_zip, PAGE_N_DIR_SLOTS, n_slots);
}
