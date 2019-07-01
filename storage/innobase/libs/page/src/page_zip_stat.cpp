#include <innodb/page/page_zip_stat.h>


/** Statistics on compression, indexed by page_zip_des_t::ssize - 1 */
page_zip_stat_t page_zip_stat[PAGE_ZIP_SSIZE_MAX];
