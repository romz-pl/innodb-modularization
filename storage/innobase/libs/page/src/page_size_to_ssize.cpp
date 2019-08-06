#include <innodb/page/page_size_to_ssize.h>

/** Convert a page size, which is a power of 2, to an ssize, which is
the number of bit shifts from 512 to make that page size.
@param[in]	page_size	compressed page size in bytes
@return an ssize created from the page size provided. */
uint32_t page_size_to_ssize(ulint page_size) {
  uint32_t ssize;

  for (ssize = UNIV_ZIP_SIZE_SHIFT_MIN;
       static_cast<uint32_t>(1 << ssize) < page_size; ssize++) {
  }

  return (ssize - UNIV_ZIP_SIZE_SHIFT_MIN + 1);
}
