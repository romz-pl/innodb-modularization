#include <innodb/page/page_zip_dir_find_free.h>

#include <innodb/assert/assert.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_find_low.h>
#include <innodb/page/page_zip_dir_size.h>
#include <innodb/page/page_zip_dir_user_size.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_simple_validate.h>

/** Find the slot of the given free record in the dense page directory.
 @return dense directory slot, or NULL if record not found */
byte *page_zip_dir_find_free(
    page_zip_des_t *page_zip, /*!< in: compressed page */
    ulint offset)             /*!< in: offset of user record */
{
  byte *end = page_zip->data + page_zip_get_size(page_zip);

  ut_ad(page_zip_simple_validate(page_zip));

  return (page_zip_dir_find_low(end - page_zip_dir_size(page_zip),
                                end - page_zip_dir_user_size(page_zip),
                                offset));
}
