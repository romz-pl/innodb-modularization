#include <innodb/page/page_zip_des_init.h>

#include <innodb/page/page_zip_des_t.h>

/** Initialize a compressed page descriptor. */
void page_zip_des_init(page_zip_des_t *page_zip) /*!< in/out: compressed page
                                                 descriptor */
{
  memset(page_zip, 0, sizeof *page_zip);
}
