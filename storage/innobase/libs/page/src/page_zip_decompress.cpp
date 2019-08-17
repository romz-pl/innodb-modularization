#include <innodb/page/page_zip_decompress.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/time/ut_time_us.h>

#include <innodb/page/page_zip_decompress_low.h>
#include <innodb/page/page_zip_stat.h>
#include <innodb/page/page_get_space_id.h>
#include <innodb/page/page_zip_stat_per_index_mutex.h>
#include <innodb/page/page_zip_stat_per_index.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_des_t.h>

#include <innodb/monitor/monitor_id_t.h>
#include <innodb/monitor/MONITOR_INC.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/buf_lru/buf_LRU_stat_inc_unzip.h>

extern bool srv_cmp_per_index_enabled;

space_index_t btr_page_get_index_id(const page_t *page);

/** Decompress a page.  This function should tolerate errors on the compressed
 page.  Instead of letting assertions fail, it will return FALSE if an
 inconsistency is detected.
 @return true on success, false on failure */
ibool page_zip_decompress(
    page_zip_des_t *page_zip, /*!< in: data, ssize;
                             out: m_start, m_end, m_nonempty, n_blobs */
    page_t *page,             /*!< out: uncompressed page, may be trashed */
    ibool all)                /*!< in: TRUE=decompress the whole page;
                              FALSE=verify but do not copy some
                              page header fields that should not change
                              after page creation */
{
#ifndef UNIV_HOTBACKUP
  uintmax_t usec = ut_time_us(NULL);
#endif /* !UNIV_HOTBACKUP */

  if (!page_zip_decompress_low(page_zip, page, all)) {
    return (FALSE);
  }

#ifndef UNIV_HOTBACKUP
  uintmax_t time_diff = ut_time_us(NULL) - usec;
  page_zip_stat[page_zip->ssize - 1].decompressed++;
  page_zip_stat[page_zip->ssize - 1].decompressed_usec += time_diff;

  if (srv_cmp_per_index_enabled) {
    index_id_t index_id(page_get_space_id(page), btr_page_get_index_id(page));

    mutex_enter(&page_zip_stat_per_index_mutex);
    page_zip_stat_per_index[index_id].decompressed++;
    page_zip_stat_per_index[index_id].decompressed_usec += time_diff;
    mutex_exit(&page_zip_stat_per_index_mutex);
  }
#endif /* !UNIV_HOTBACKUP */

  /* Update the stat counter for LRU policy. */
  buf_LRU_stat_inc_unzip();

  MONITOR_INC(MONITOR_PAGE_DECOMPRESS);

  return (TRUE);
}
