/*****************************************************************************

Copyright (c) 2017, 2019, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/arch0arch.h
 Common interface for redo log and dirty page archiver system

 *******************************************************/

#ifndef ARCH_ARCH_INCLUDE
#define ARCH_ARCH_INCLUDE

#include <innodb/univ/univ.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/error/dberr_t.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/log_archiver_thread_event.h>
#include <innodb/log_arch/log_archiver_is_active.h>
#include <innodb/log_arch/Arch_Client_State.h>
#include <innodb/log_arch/Arch_State.h>
#include <innodb/log_arch/Arch_Blk_State.h>
#include <innodb/log_arch/Arch_Blk_Type.h>
#include <innodb/log_arch/Arch_Blk_Flush_Type.h>
#include <innodb/log_arch/Arch_Page_Dblwr_Offset.h>
#include <innodb/log_arch/Arch_Page_Pos.h>
#include <innodb/log_arch/Arch_Point.h>
#include <innodb/log_arch/Arch_Reset_File.h>
#include <innodb/log_arch/Arch_Reset.h>
#include <innodb/log_arch/Arch_Block.h>
#include <innodb/log_arch/Arch_File_Ctx.h>
#include <innodb/log_arch/Arch_Group.h>


#include <mysql/components/services/page_track_service.h>
#include "log0log.h"

#include <list>
#include <deque>

class buf_page_t;



/** Log archiver background thread */
void log_archiver_thread();

/** Remove files related to page and log archiving.
@param[in]	file_path	path to the file
@param[in]	file_name	name of the file */
void arch_remove_file(const char *file_path, const char *file_name);

/** Remove group directory and the files related to page and log archiving.
@param[in]	dir_path	path to the directory
@param[in]	dir_name	directory name */
void arch_remove_dir(const char *dir_path, const char *dir_name);

/** Initialize Page and Log archiver system
@return error code */
dberr_t arch_init();

/** Free Page and Log archiver system */
void arch_free();

/** Start log archiver background thread.
@return error code */
int start_log_archiver_background();

/** Start page archiver background thread.
@return error code */
int start_page_archiver_background();

/** Archiver thread event to signal that data is available */
extern os_event_t page_archiver_thread_event;

/** Global to indicate if page archiver thread is active. */
extern bool page_archiver_is_active;

/** Page archiver background thread */
void page_archiver_thread();

/** Forward declarations */
class Arch_Group;
class Arch_Log_Sys;
class Arch_Dblwr_Ctx;
struct Arch_Recv_Group_Info;





/** A list of archive groups */
using Arch_Grp_List = std::list<Arch_Group *, ut_allocator<Arch_Group *>>;

/** An iterator for archive group */
using Arch_Grp_List_Iter = Arch_Grp_List::iterator;

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

/** Vector of page archive in memory blocks */
using Arch_Block_Vec = std::vector<Arch_Block *, ut_allocator<Arch_Block *>>;

/** Page archiver in memory data */
struct ArchPageData {
  /** Constructor */
  ArchPageData() {}

  /** Allocate buffer and initialize blocks
  @return true, if successful */
  bool init();

  /** Delete blocks and buffer */
  void clean();

  /** Get the block for a position
  @param[in]	pos	position in page archive sys
  @param[in]	type	block type
  @return page archive in memory block */
  Arch_Block *get_block(Arch_Page_Pos *pos, Arch_Blk_Type type);

  /** @return temporary block used to copy active block for partial flush. */
  Arch_Block *get_partial_flush_block() const {
    return (m_partial_flush_block);
  }

  /** Vector of data blocks */
  Arch_Block_Vec m_data_blocks{};

  /** Reset block */
  Arch_Block *m_reset_block{nullptr};

  /** Temporary block used to copy active block for partial flush. */
  Arch_Block *m_partial_flush_block{nullptr};

  /** Block size in bytes */
  uint m_block_size{};

  /** Total number of blocks */
  uint m_num_data_blocks{};

  /** In memory buffer */
  byte *m_buffer{nullptr};
};

/** Forward declaration. */
class Page_Arch_Client_Ctx;

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

/** Redo log archiver system global */
extern Arch_Log_Sys *arch_log_sys;

/** Dirty page ID archiver system global */
extern Arch_Page_Sys *arch_page_sys;

#endif /* ARCH_ARCH_INCLUDE */
