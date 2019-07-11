#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_no_t.h>

/** File space extent size in pages
page size | file space extent size
----------+-----------------------
   4 KiB  | 256 pages = 1 MiB
   8 KiB  | 128 pages = 1 MiB
  16 KiB  |  64 pages = 1 MiB
  32 KiB  |  64 pages = 2 MiB
  64 KiB  |  64 pages = 4 MiB
*/
#define FSP_EXTENT_SIZE                                                 \
  static_cast<page_no_t>(                                               \
      ((UNIV_PAGE_SIZE <= (16384)                                       \
            ? (1048576 / UNIV_PAGE_SIZE)                                \
            : ((UNIV_PAGE_SIZE <= (32768)) ? (2097152 / UNIV_PAGE_SIZE) \
                                           : (4194304 / UNIV_PAGE_SIZE)))))

/** File space extent size (four megabyte) in pages for MAX page size */
#define FSP_EXTENT_SIZE_MAX (4194304 / UNIV_PAGE_SIZE_MAX)

/** File space extent size (one megabyte) in pages for MIN page size */
#define FSP_EXTENT_SIZE_MIN (1048576 / UNIV_PAGE_SIZE_MIN)
