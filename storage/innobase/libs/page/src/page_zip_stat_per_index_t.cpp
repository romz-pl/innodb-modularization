#include <innodb/page/page_zip_stat_per_index_t.h>


#ifndef UNIV_HOTBACKUP

/** Statistics on compression, indexed by index->id */
page_zip_stat_per_index_t page_zip_stat_per_index;

#endif /* !UNIV_HOTBACKUP */
