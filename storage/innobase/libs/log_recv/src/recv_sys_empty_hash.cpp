#include <innodb/log_recv/recv_sys_empty_hash.h>

#include <innodb/allocator/UT_NEW.h>
#include <innodb/log_recv/psf.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/logger/fatal.h>
#include <innodb/memory/mem_heap_free.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Empties the hash table when it has been fully processed. */
void recv_sys_empty_hash() {
  ut_ad(mutex_own(&recv_sys->mutex));

  if (recv_sys->n_addrs != 0) {
    ib::fatal(ER_IB_MSG_699, ulonglong{recv_sys->n_addrs});
  }

  for (auto &space : *recv_sys->spaces) {
    if (space.second.m_heap != nullptr) {
      mem_heap_free(space.second.m_heap);
      space.second.m_heap = nullptr;
    }
  }

  UT_DELETE(recv_sys->spaces);

  using Spaces = recv_sys_t::Spaces;

  recv_sys->spaces = UT_NEW(Spaces(), mem_log_recv_space_hash_key);
}
