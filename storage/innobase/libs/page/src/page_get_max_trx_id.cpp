#include <innodb/page/page_get_max_trx_id.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/page/header.h>

/** Returns the max trx id field value. */
trx_id_t page_get_max_trx_id(const page_t *page) /*!< in: page */
{
  ut_ad(page);

  return (mach_read_from_8(page + PAGE_HEADER + PAGE_MAX_TRX_ID));
}
