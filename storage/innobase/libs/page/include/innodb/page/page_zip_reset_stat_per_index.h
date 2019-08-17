#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_zip_stat_per_index_mutex.h>
#include <innodb/page/page_zip_stat_per_index.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

#ifndef UNIV_HOTBACKUP
/** Reset the counters used for filling
 INFORMATION_SCHEMA.innodb_cmp_per_index. */
UNIV_INLINE
void page_zip_reset_stat_per_index() {
  mutex_enter(&page_zip_stat_per_index_mutex);

  page_zip_stat_per_index.erase(page_zip_stat_per_index.begin(),
                                page_zip_stat_per_index.end());

  mutex_exit(&page_zip_stat_per_index_mutex);
}
#endif /* !UNIV_HOTBACKUP */
