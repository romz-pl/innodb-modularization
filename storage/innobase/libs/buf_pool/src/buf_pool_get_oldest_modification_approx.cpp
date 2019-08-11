#include <innodb/buf_pool/buf_pool_get_oldest_modification_approx.h>

#include <innodb/buf_pool/srv_buf_pool_instances.h>
#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/buf_pool/buf_pool_from_array.h>
#include <innodb/buffer/buf_flush_list_mutex_enter.h>
#include <innodb/buffer/buf_flush_list_mutex_exit.h>
#include <innodb/tablespace/fsp_is_system_temporary.h>

lsn_t buf_pool_get_oldest_modification_approx(void) {
  lsn_t lsn = 0;
  lsn_t oldest_lsn = 0;

  /* When we traverse all the flush lists we don't care if previous
  flush lists changed. We do not require consistent result. */

  for (ulint i = 0; i < srv_buf_pool_instances; i++) {
    buf_pool_t *buf_pool;

    buf_pool = buf_pool_from_array(i);

    buf_flush_list_mutex_enter(buf_pool);

    buf_page_t *bpage;

    /* We don't let log-checkpoint halt because pages from system
    temporary are not yet flushed to the disk. Anyway, object
    residing in system temporary doesn't generate REDO logging. */
    for (bpage = UT_LIST_GET_LAST(buf_pool->flush_list);
         bpage != NULL && fsp_is_system_temporary(bpage->id.space());
         bpage = UT_LIST_GET_PREV(list, bpage)) {
      /* Do nothing. */
    }

    if (bpage != NULL) {
      ut_ad(bpage->in_flush_list);
      lsn = bpage->oldest_modification;
    }

    buf_flush_list_mutex_exit(buf_pool);

    if (!oldest_lsn || oldest_lsn > lsn) {
      oldest_lsn = lsn;
    }
  }

  /* The returned answer may be out of date: the flush_list can
  change after the mutex has been released. */

  return (oldest_lsn);
}

