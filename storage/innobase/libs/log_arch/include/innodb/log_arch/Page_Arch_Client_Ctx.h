#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Page_Pos.h>
#include <innodb/log_arch/Arch_Client_State.h>
#include <innodb/log_types/lsn_t.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/log_arch/Page_Arch_Cbk.h>
#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_create.h>

class Arch_Group;

/** Dirty page archiver client context */
class Page_Arch_Client_Ctx {
 public:
  /** Constructor: Initialize elements
  @param[in]	is_durable	true if the client requires durability, else
  false */
  Page_Arch_Client_Ctx(bool is_durable) : m_is_durable(is_durable) {
    m_start_pos.init();
    m_stop_pos.init();
    mutex_create(LATCH_ID_PAGE_ARCH_CLIENT, &m_mutex);
  }

  /** Destructor. */
  ~Page_Arch_Client_Ctx() { mutex_free(&m_mutex); }

  /** Start dirty page tracking and archiving
  @param[in]	recovery	true if the tracking is being started as part of
  recovery process
  @param[out]   start_id    fill the start lsn
  @return error code. */
  int start(bool recovery, uint64_t *start_id);

  /** Stop dirty page tracking and archiving
  @param[out]	stop_id	fill the stop lsn
  @return error code. */
  int stop(uint64_t *stop_id);

  /** Release archived data so that system can purge it */
  void release();

  /** Initialize context during recovery.
  @param[in]	group		Group which needs to be attached to the client
  @param[in]	last_lsn	last reset lsn
  @return error code. */
  int init_during_recovery(Arch_Group *group, lsn_t last_lsn);

  /** Check if this client context is active.
  @return true if active, else false */
  bool is_active() const { return (m_state == ARCH_CLIENT_STATE_STARTED); }

  /** Get archived page Ids.
  Attempt to read blocks directly from in memory buffer. If overwritten,
  copy from archived files.
  @param[in]	cbk_func	called repeatedly with page ID buffer
  @param[in]	cbk_ctx		callback function context
  @param[in,out]	buff		buffer to fill page IDs
  @param[in]	buf_len		buffer length in bytes
  @return error code */
  int get_pages(Page_Arch_Cbk *cbk_func, void *cbk_ctx, byte *buff,
                uint buf_len);

#ifdef UNIV_DEBUG
  /** Print information related to the archiver client for debugging purposes.
   */
  void print();
#endif

  /** Disable copy construction */
  Page_Arch_Client_Ctx(Page_Arch_Client_Ctx const &) = delete;

  /** Disable assignment */
  Page_Arch_Client_Ctx &operator=(Page_Arch_Client_Ctx const &) = delete;

 private:
  /** Acquire client archiver mutex.
  It synchronizes members on concurrent start and stop operations. */
  void arch_client_mutex_enter() { mutex_enter(&m_mutex); }

  /** Release client archiver mutex */
  void arch_client_mutex_exit() { mutex_exit(&m_mutex); }

 private:
  /** Page archiver client state */
  Arch_Client_State m_state{ARCH_CLIENT_STATE_INIT};

  /** Archive group the client is attached to */
  Arch_Group *m_group{nullptr};

  /** True if the client requires durablity */
  bool m_is_durable;

  /** Start LSN for archived data */
  lsn_t m_start_lsn{LSN_MAX};

  /** Stop LSN for archived data */
  lsn_t m_stop_lsn{LSN_MAX};

  /** Reset LSN at the time of last reset. */
  lsn_t m_last_reset_lsn{LSN_MAX};

  /** Start position for client in archived file group */
  Arch_Page_Pos m_start_pos;

  /** Stop position for client in archived file group */
  Arch_Page_Pos m_stop_pos;

  /** Mutex protecting concurrent operation on data */
  ib_mutex_t m_mutex;
};
