#include <innodb/io/Compression.h>

#include <innodb/machine/data.h>
#include <innodb/page/page_type_t.h>

/** @return true if it is a compressed page */
bool Compression::is_compressed_page(const byte *page) {
  return (mach_read_from_2(page + FIL_PAGE_TYPE) == FIL_PAGE_COMPRESSED);
}
