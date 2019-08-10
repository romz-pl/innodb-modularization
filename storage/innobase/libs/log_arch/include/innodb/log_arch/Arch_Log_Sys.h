#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/log_arch/Arch_Grp_List.h>
#include <innodb/log_arch/Arch_State.h>
#include <innodb/log_sn/atomic_lsn_t.h>
#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_mutex/mutex_free.h>

class Arch_Group;
class Arch_File_Ctx;

/** Redo log archiving system */
class Arch_Log_Sys {
 public:
  /** Constructor: Initialize members */
  Arch_Log_Sys()
      : m_state(ARCH_STATE_INIT),
        m_archived_lsn(LSN_MAX),
        m_group_list(),
        m_current_group() {
    mutex_create(LATCH_ID_LOG_ARCH, &m_mutex);
  }

  /** Destructor: Free mutex */
  ~Arch_Log_Sys() {
    ut_ad(m_state == ARCH_STATE_INIT || m_state == ARCH_STATE_ABORT);
    ut_ad(m_current_group == nullptr);
    ut_ad(m_group_list.empty());

    mutex_free(&m_mutex);
  }

  /** Check if archiving is in progress.
  In #ARCH_STATE_PREPARE_IDLE state, all clients have already detached
  but archiver background task is yet to finish.
  @return true, if archiving is active */
  bool is_active() {
    return (m_state == ARCH_STATE_ACTIVE || m_state == ARCH_STATE_PREPARE_IDLE);
  }

  /** Check if archiver system is in initial state
  @return true, if redo log archiver state is #ARCH_STATE_INIT */
  bool is_init() { return (m_state == ARCH_STATE_INIT); }

  /** Get LSN up to which redo is archived
  @return last archived redo LSN */
  lsn_t get_archived_lsn() { return (m_archived_lsn.load()); }

  /** Get current redo log archive group
  @return current archive group */
  Arch_Group *get_arch_group() { return (m_current_group); }

  /** Start redo log archiving.
  If archiving is already in progress, the client
  is attached to current group.
  @param[out]	group		log archive group
  @param[out]	start_lsn	start lsn for client
  @param[out]	header		redo log header
  @param[in]	is_durable	if client needs durable archiving
  @return error code */
  int start(Arch_Group *&group, lsn_t &start_lsn, byte *header,
            bool is_durable);

  /** Stop redo log archiving.
  If other clients are there, the client is detached from
  the current group.
  @param[out]	group		log archive group
  @param[out]	stop_lsn	stop lsn for client
  @param[out]	log_blk		redo log trailer block
  @param[in,out]	blk_len		length in bytes
  @return error code */
  int stop(Arch_Group *group, lsn_t &stop_lsn, byte *log_blk,
           uint32_t &blk_len);

  /** Force to abort the archiver (state becomes ARCH_STATE_ABORT). */
  void force_abort();

  /** Release the current group from client.
  @param[in]	group		group the client is attached to
  @param[in]	is_durable	if client needs durable archiving */
  void release(Arch_Group *group, bool is_durable);

  /** Archive accumulated redo log in current group.
  This interface is for archiver background task to archive redo log
  data by calling it repeatedly over time.
  @param[in] init		true when called for first time; it will then
                                be set to false
  @param[in]	curr_ctx	system redo logs to copy data from
  @param[out]	arch_lsn	LSN up to which archiving is completed
  @param[out]	wait		true, if no more redo to archive
  @return true, if archiving is aborted */
  bool archive(bool init, Arch_File_Ctx *curr_ctx, lsn_t *arch_lsn, bool *wait);

  /** Acquire redo log archiver mutex.
  It synchronizes concurrent start and stop operations by
  multiple clients. */
  void arch_mutex_enter() { mutex_enter(&m_mutex); }

  /** Release redo log archiver mutex */
  void arch_mutex_exit() { mutex_exit(&m_mutex); }

  /** Disable copy construction */
  Arch_Log_Sys(Arch_Log_Sys const &) = delete;

  /** Disable assignment */
  Arch_Log_Sys &operator=(Arch_Log_Sys const &) = delete;

 private:
  /** Wait for redo log archive up to the target LSN.
  We need to wait till current log sys LSN during archive stop.
  @param[in]	target_lsn	target archive LSN to wait for
  @return error code */
  int wait_archive_complete(lsn_t target_lsn);

  /** Update checkpoint LSN and related information in redo
  log header block.
  @param[in,out]	header		redo log header buffer
  @param[in]	checkpoint_lsn	checkpoint LSN for recovery */
  void update_header(byte *header, lsn_t checkpoint_lsn);

  /** Check and set log archive system state and output the
  amount of redo log available for archiving.
  @param[in]	is_abort	need to abort
  @param[in,out]	archived_lsn	LSN up to which redo log is archived
  @param[out]	to_archive	amount of redo log to be archived */
  Arch_State check_set_state(bool is_abort, lsn_t *archived_lsn,
                             uint *to_archive);

  /** Copy redo log from file context to archiver files.
  @param[in]	file_ctx	file context for system redo logs
  @param[in]	length		data to copy in bytes
  @return error code */
  dberr_t copy_log(Arch_File_Ctx *file_ctx, uint length);

 private:
  /** Mutex to protect concurrent start, stop operations */
  ib_mutex_t m_mutex;

  /** Archiver system state.
  #m_state is protected by #m_mutex and #log_t::writer_mutex. For changing
  the state both needs to be acquired. For reading, hold any of the two
  mutexes. Same is true for #m_archived_lsn. */
  Arch_State m_state;

  /** System has archived log up to this LSN */
  atomic_lsn_t m_archived_lsn;

  /** List of log archive groups */
  Arch_Grp_List m_group_list;

  /** Current archive group */
  Arch_Group *m_current_group;

  /** Chunk size to copy redo data */
  uint m_chunk_size;

  /** System log file number where the archiving started */
  uint m_start_log_index;

  /** System log file offset where the archiving started */
  ib_uint64_t m_start_log_offset;
};
