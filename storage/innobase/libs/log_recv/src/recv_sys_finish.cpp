#include <innodb/log_recv/recv_sys_finish.h>

#include <innodb/log_recv/recv_sys.h>
#include <innodb/memory/mem_heap_free.h>

/** Free up recovery data structures. */
void recv_sys_finish() {
  if (recv_sys->spaces != nullptr) {
    for (auto &space : *recv_sys->spaces) {
      if (space.second.m_heap != nullptr) {
        mem_heap_free(space.second.m_heap);
        space.second.m_heap = nullptr;
      }
    }

    UT_DELETE(recv_sys->spaces);
  }

#ifndef UNIV_HOTBACKUP
  ut_a(recv_sys->dblwr.pages.empty());

  if (!recv_sys->dblwr.deferred.empty()) {
    /* Free the pages that were not required for recovery. */
    for (auto &page : recv_sys->dblwr.deferred) {
      page.close();
    }
  }

  recv_sys->dblwr.deferred.clear();
#endif /* !UNIV_HOTBACKUP */

  ut_free(recv_sys->buf);
  ut_free(recv_sys->last_block_buf_start);
  UT_DELETE(recv_sys->metadata_recover);

  recv_sys->buf = nullptr;
  recv_sys->spaces = nullptr;
  recv_sys->metadata_recover = nullptr;
  recv_sys->last_block_buf_start = nullptr;
}
