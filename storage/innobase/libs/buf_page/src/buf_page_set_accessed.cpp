#include <innodb/buf_page/buf_page_set_accessed.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_get_mutex.h>
#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buf_page/buf_page_in_file.h>
#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/time/ut_time_ms.h>

/** Flag a block accessed. */
void buf_page_set_accessed(buf_page_t *bpage) /*!< in/out: control block */
{
  ut_ad(mutex_own(buf_page_get_mutex(bpage)));

  ut_a(buf_page_in_file(bpage));

  if (bpage->access_time == 0) {
    /* Make this the time of the first access. */
    bpage->access_time = static_cast<uint>(ut_time_ms());
  }
}
#endif /* !UNIV_HOTBACKUP */
