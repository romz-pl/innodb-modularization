#include <innodb/io/os_file_original_page_size.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/disk/page_type_t.h>
#include <innodb/disk/flags.h>

/** If it is a compressed page return the original page data + footer size
@param[in] buf		Buffer to check, must include header + 10 bytes
@return ULINT_UNDEFINED if the page is not a compressed page or length
        of the original data + footer if it is a compressed page */
ulint os_file_original_page_size(const byte *buf) {
  ulint type = mach_read_from_2(buf + FIL_PAGE_TYPE);

  if (type == FIL_PAGE_COMPRESSED) {
    ulint version = mach_read_from_1(buf + FIL_PAGE_VERSION);
    ut_a(version == 1);

    return (mach_read_from_2(buf + FIL_PAGE_ORIGINAL_SIZE_V1));
  }

  return (ULINT_UNDEFINED);
}
