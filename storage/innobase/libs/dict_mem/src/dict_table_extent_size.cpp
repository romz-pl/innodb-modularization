#include <innodb/dict_mem/dict_table_extent_size.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_page_size.h>

/** Determine the extent size (in pages) for the given table
@param[in]	table	the table whose extent size is being
                        calculated.
@return extent size in pages (256, 128 or 64) */
page_no_t dict_table_extent_size(const dict_table_t *table) {
  const ulint mb_1 = 1024 * 1024;
  const ulint mb_2 = 2 * mb_1;
  const ulint mb_4 = 4 * mb_1;

  page_size_t page_size = dict_table_page_size(table);
  page_no_t pages_in_extent = FSP_EXTENT_SIZE;

  if (page_size.is_compressed()) {
    ulint disk_page_size = page_size.physical();

    switch (disk_page_size) {
      case 1024:
        pages_in_extent = mb_1 / 1024;
        break;
      case 2048:
        pages_in_extent = mb_1 / 2048;
        break;
      case 4096:
        pages_in_extent = mb_1 / 4096;
        break;
      case 8192:
        pages_in_extent = mb_1 / 8192;
        break;
      case 16384:
        pages_in_extent = mb_1 / 16384;
        break;
      case 32768:
        pages_in_extent = mb_2 / 32768;
        break;
      case 65536:
        pages_in_extent = mb_4 / 65536;
        break;
      default:
        ut_ad(0);
    }
  }

  return (pages_in_extent);
}
