#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/log_arch/ArchPageData.h>
#include <innodb/log_arch/Arch_Group.h>
#include <innodb/log_arch/Arch_Grp_List.h>
#include <innodb/log_arch/Arch_Page_Pos.h>
#include <innodb/log_arch/Arch_State.h>
#include <innodb/log_sn/lsn_t.h>
#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <mysql/components/services/page_track_service.h>

#include <vector>
#include <utility>

class Page_Arch_Client_Ctx;
class Arch_Group;
struct Arch_Page_Pos;
class buf_page_t;

/** Dirty page archive system */
class Arch_Page_Sys {
 public:
  /** Constructor: Initialize elements and create mutex */
  Arch_Page_Sys();

  /** Destructor: Free memory buffer and mutexes */
  ~Arch_Page_Sys();

  /** Start dirty page ID archiving.
  If archiving is already in progress, the client is attached to current group.
  @param[out]	group		page archive group the client gets attached to
  @param[out]	start_lsn	start lsn for client in archived data
  @param[out]	start_pos	start position for client in archived data
  @param[in]	is_durable	true if client needs durable archiving
  @param[in]	restart	true if client is already attached to current group
  @param[in]	recovery	true if archiving is being started during
  recovery
  @return error code */
  int start(Arch_Group **group, lsn_t *start_lsn, Arch_Page_Pos *start_pos,
            bool is_durable, bool restart, bool recovery);

  /** Stop dirty page ID archiving.
  If other clients are there, the client is detached from the current group.
  @param[in]	group		page archive group the client is attached to
  @param[out]	stop_lsn	stop lsn for client
  @param[out]	stop_pos	stop position in archived data
  @param[in]	is_durable	true if client needs durable archiving
  @return error code */
  int stop(Arch_Group *group, lsn_t *stop_lsn, Arch_Page_Pos *stop_pos,
           bool is_durable);

  /** Start dirty page ID archiving during recovery.
  @param[in]	group	Group which needs to be attached to the archiver
  @param[in]	new_empty_file  true if there was a empty file created
  @return error code */
  int start_during_recovery(Arch_Group *group, bool new_empty_file);

  /** Release the current group from client.
  @param[in]	group		group the client is attached to
  @param[in]	is_durable	if client needs durable archiving
  @param[in]	start_pos	start position when the client calling the
  release was started */
  void release(Arch_Group *group, bool is_durable, Arch_Page_Pos start_pos);

  /** Check and add page ID to archived data.
  Check for duplicate page.
  @param[in]	bpage		page to track
  @param[in]	track_lsn	LSN when tracking started
  @param[in]	frame_lsn	current LSN of the page
  @param[in]	force		if true, add page ID without check */
  void track_page(buf_page_t *bpage, lsn_t track_lsn, lsn_t frame_lsn,
                  bool force);

  /** Flush all the unflushed inactive blocks and flush the active block if
  required.
  @note Used only during the checkpointing process.
  @param[in]	checkpoint_lsn	next checkpoint LSN */
  void flush_at_checkpoint(lsn_t checkpoint_lsn);

  /** Archive dirty page IDs in current group.
  This interface is for archiver background task to flush page archive
  data to disk by calling it repeatedly over time.
  @param[out]	wait	true, if no more data to archive
  @return true, if archiving is aborted */
  bool archive(bool *wait);

  /** Acquire dirty page ID archiver mutex.
  It synchronizes concurrent start and stop operations by multiple clients. */
  void arch_mutex_enter() { mutex_enter(&m_mutex); }

  /** Release page ID archiver mutex */
  void arch_mutex_exit() { mutex_exit(&m_mutex); }

  /** Acquire dirty page ID archive operation mutex.
  It synchronizes concurrent page ID write to memory buffer. */
  void arch_oper_mutex_enter() { mutex_enter(&m_oper_mutex); }

  /** Release page ID archiver operatiion  mutex */
  void arch_oper_mutex_exit() { mutex_exit(&m_oper_mutex); }

  /* Save information at the time of a reset considered as the reset point.
  @return error code */
  void save_reset_point(bool is_durable);

  /** Wait for reset info to be flushed to disk.
  @param[in]	request_block	block number until which blocks need to be
  flushed
  @return true if flushed, else false */
  bool wait_for_reset_info_flush(uint64_t request_block);

  /** Get the group which has tracked pages between the start_id and stop_id.
  @param[in,out]	start_id	start LSN from which tracked pages are
  required; updated to the actual start LSN used for the search
  @param[in,out]	stop_id     stop_lsn until when tracked pages are
  required; updated to the actual stop LSN used for the search
  @param[out]		group       group which has the required tracked
  pages, else nullptr.
  @return error */
  int fetch_group_within_lsn_range(lsn_t &start_id, lsn_t &stop_id,
                                   Arch_Group **group);

  /** Purge the archived files until the specified purge LSN.
  @param[in]	purge_lsn	purge lsn until where files needs to be purged
  @return error code
  @retval 0 if purge was successful */
  uint purge(lsn_t *purge_lsn);

  /** Update the stop point in all the required structures.
  @param[in]	cur_blk	block which needs to be updated with the stop info */
  void update_stop_info(Arch_Block *cur_blk);

  /** Fetch the status of the page tracking system.
  @param[out]	status	vector of a pair of (ID, bool) where ID is the
  start/stop point and bool is true if the ID is a start point else false */
  void get_status(std::vector<std::pair<lsn_t, bool>> &status) {
    for (auto group : m_group_list) {
      group->get_status(status);
    }
  }

  /** Given start and stop position find number of pages tracked between them
  @param[in]	start_pos	start position
  @param[in]	stop_pos	stop position
  @param[out]	num_pages	number of pages tracked between start and stop
  position
  @return false if start_pos and stop_pos are invalid else true */
  bool get_num_pages(Arch_Page_Pos start_pos, Arch_Page_Pos stop_pos,
                     uint64_t &num_pages);

  /** Get approximate number of tracked pages between two given LSN values.
  @param[in,out]      start_id        fetch archived page Ids from this LSN
  @param[in,out]      stop_id         fetch archived page Ids until this LSN
  @param[out]         num_pages       number of pages tracked between specified
  LSN range
  @return error code */
  int get_num_pages(lsn_t &start_id, lsn_t &stop_id, uint64_t *num_pages);

  /** Get page IDs from a specific position.
  Caller must ensure that read_len doesn't exceed the block.
  @param[in]	group		group whose pages we're interested in
  @param[in]	read_pos	position in archived data
  @param[in]	read_len	amount of data to read
  @param[out]	read_buff	buffer to return the page IDs.
  @note Caller must allocate the buffer.
  @return true if we could successfully read the block. */
  bool get_pages(Arch_Group *group, Arch_Page_Pos *read_pos, uint read_len,
                 byte *read_buff);

  /** Get archived page Ids between two given LSN values.
  Attempt to read blocks directly from in memory buffer. If overwritten,
  copy from archived files.
  @param[in]	thd		thread handle
  @param[in]      cbk_func        called repeatedly with page ID buffer
  @param[in]      cbk_ctx         callback function context
  @param[in,out]  start_id        fetch archived page Ids from this LSN
  @param[in,out]  stop_id         fetch archived page Ids until this LSN
  @param[in]      buf             buffer to fill page IDs
  @param[in]      buf_len         buffer length in bytes
  @return error code */
  int get_pages(MYSQL_THD thd, Page_Track_Callback cbk_func, void *cbk_ctx,
                lsn_t &start_id, lsn_t &stop_id, byte *buf, uint buf_len);

  /** Set the latest stop LSN to the checkpoint LSN at the time it's called. */
  void post_recovery_init();

  /** Recover the archiver system at the time of startup. Recover information
  related to all the durable groups and start archiving if any group was active
  at the time of crash/shutdown.
  @return error code */
  dberr_t recover();

#ifdef UNIV_DEBUG
  /** Print information related to the archiver for debugging purposes. */
  void print();
#endif

  /** Check if archiver system is in initial state
  @return true, if page ID archiver state is #ARCH_STATE_INIT */
  bool is_init() const { return (m_state == ARCH_STATE_INIT); }

  /** Check if archiver system is active
  @return true, if page ID archiver state is #ARCH_STATE_ACTIVE or
  #ARCH_STATE_PREPARE_IDLE. */
  bool is_active() const {
    return (m_state == ARCH_STATE_ACTIVE || m_state == ARCH_STATE_PREPARE_IDLE);
  }

  /** @return true if in abort state */
  bool is_abort() const { return (m_state == ARCH_STATE_ABORT); }

  /** Get the mutex protecting concurrent start, stop operations required
  for initialising group during recovery.
  @return mutex */
  ib_mutex_t *get_mutex() { return (&m_mutex); }

  /** @return operation mutex */
  ib_mutex_t *get_oper_mutex() { return (&m_oper_mutex); }

  /** Fetch the system client context.
  @return system client context. */
  Page_Arch_Client_Ctx *get_sys_client() const { return (m_ctx); }

  /** @return the latest stop LSN */
  lsn_t get_latest_stop_lsn() const { return (m_latest_stop_lsn); }

  /** Disable copy construction */
  Arch_Page_Sys(Arch_Page_Sys const &) = delete;

  /** Disable assignment */
  Arch_Page_Sys &operator=(Arch_Page_Sys const &) = delete;

  class Recv;

 private:
  /** Wait for archive system to come out of #ARCH_STATE_PREPARE_IDLE.
  If the system is preparing to idle, #start needs to wait
  for it to come to idle state.
  @return true, if successful
          false, if needs to abort */
  bool wait_idle();

  /** Check if the gap from last reset is short.
  If not many page IDs are added till last reset, we avoid
  taking a new reset point
  @return true, if the gap is small. */
  bool is_gap_small();

  /** Enable tracking pages in all buffer pools.
  @param[in]	tracking_lsn	track pages from this LSN */
  void set_tracking_buf_pool(lsn_t tracking_lsn);

  /** Track pages for which IO is already started. */
  void track_initial_pages();

  /** Flush the blocks to disk.
  @param[out]	wait	true, if no more data to archive
  @return error code */
  dberr_t flush_blocks(bool *wait);

  /** Flush all the blocks which are ready to be flushed but not flushed.
  @param[out]	cur_pos	position of block which needs to be flushed
  @param[in]	end_pos	position of block until which the blocks need to
  be flushed
  @return error code */
  dberr_t flush_inactive_blocks(Arch_Page_Pos &cur_pos, Arch_Page_Pos end_pos);

  /** Do a partial flush of the current active block
  @param[in]	cur_pos	position of block which needs to be flushed
  @param[in]	partial_reset_block_flush	true if reset block needs to be
  flushed
  @return error code */
  dberr_t flush_active_block(Arch_Page_Pos cur_pos,
                             bool partial_reset_block_flush);

 private:
  /** Mutex protecting concurrent start, stop operations */
  ib_mutex_t m_mutex;

  /** Archiver system state. */
  Arch_State m_state{ARCH_STATE_INIT};

  /** List of log archive groups */
  Arch_Grp_List m_group_list{};

  /** Position where last client started archiving */
  Arch_Page_Pos m_last_pos{};

  /** LSN when last client started archiving */
  lsn_t m_last_lsn{LSN_MAX};

  /** Latest LSN until where the tracked pages have been flushed. */
  lsn_t m_latest_stop_lsn{LSN_MAX};

  /** LSN until where the groups are purged. */
  lsn_t m_latest_purged_lsn{LSN_MAX};

  /** Mutex protecting concurrent operation on data */
  ib_mutex_t m_oper_mutex;

  /** Current archive group */
  Arch_Group *m_current_group{nullptr};

  /** In memory data buffer */
  ArchPageData m_data{};

  /** Position to add new page ID */
  Arch_Page_Pos m_write_pos{};

  /** Position to add new reset element */
  Arch_Page_Pos m_reset_pos{};

  /** Position set to explicitly request the flush archiver to flush until
  this position.
  @note this is always increasing and is only updated by the requester thread
  like checkpoint */
  Arch_Page_Pos m_request_flush_pos{};

  /** Block number set to explicitly request the flush archiver to partially
  flush the current active block with reset LSN.
  @note this is always increasing and is only updated by the requester thread
  like checkpoint */
  uint64_t m_request_blk_num_with_lsn{std::numeric_limits<uint64_t>::max()};

  /** Block number set once the flush archiver partially flushes the current
  active block with reset LSN.
  @note this is always increasing and is only updated by the requester thread
  like checkpoint */
  uint64_t m_flush_blk_num_with_lsn{std::numeric_limits<uint64_t>::max()};

  /** Position for start flushing
  @note this is always increasing and is only updated by the page archiver
  thread */
  Arch_Page_Pos m_flush_pos{};

  /** The index of the file the last reset belonged to.  */
  uint m_last_reset_file_index{0};

  /** System client. */
  Page_Arch_Client_Ctx *m_ctx;
};
