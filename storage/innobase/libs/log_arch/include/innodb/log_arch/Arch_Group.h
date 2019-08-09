#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Blk_Type.h>
#include <innodb/log_arch/Arch_Block.h>
#include <innodb/log_arch/Arch_File_Ctx.h>
#include <innodb/log_arch/Arch_Page_Dblwr_Offset.h>
#include <innodb/sync_mutex/ib_mutex_t.h>

struct Arch_Recv_Group_Info;
class Arch_Dblwr_Ctx;

/** Contiguous archived data for redo log or page tracking.
If there is a gap, that is if archiving is stopped and started, a new
group is created. */
class Arch_Group {
 public:
  /** Constructor: Initialize members
  @param[in]	start_lsn	start LSN for the group
  @param[in]	header_len	length of header for archived files
  @param[in]	mutex		archive system mutex from caller */
  Arch_Group(lsn_t start_lsn, uint header_len, ib_mutex_t *mutex)
      : m_begin_lsn(start_lsn),
        m_header_len(header_len)
#ifdef UNIV_DEBUG
        ,
        m_arch_mutex(mutex)
#endif /* UNIV_DEBUG */
  {
    m_active_file.m_file = OS_FILE_CLOSED;
    m_durable_file.m_file = OS_FILE_CLOSED;
    m_stop_pos.init();
  }

  /** Destructor: Delete all files for non-durable archiving. */
  ~Arch_Group();

  /** Initialize the doublewrite buffer file context for the archive group.
  @param[in]	path		path to the file
  @param[in]	base_file	file name prefix
  @param[in]	num_files	initial number of files
  @param[in]	file_size	file size in bytes
  @return error code. */
  static dberr_t init_dblwr_file_ctx(const char *path, const char *base_file,
                                     uint num_files, uint64_t file_size);

  /** Initialize the file context for the archive group.
  File context keeps the archived data in files on disk. There
  is one file context for a archive group.
  @param[in]	path			path to the file
  @param[in]	base_dir		directory name prefix
  @param[in]	base_file		file name prefix
  @param[in]	num_files		initial number of files
  @param[in]	file_size		file size in bytes
  @return error code. */
  dberr_t init_file_ctx(const char *path, const char *base_dir,
                        const char *base_file, uint num_files,
                        uint64_t file_size) {
    return (m_file_ctx.init(path, base_dir, base_file, num_files, file_size));
  }

  /* Close the file contexts when they're not required anymore. */
  void close_file_ctxs() {
    m_file_ctx.close();

    if (m_durable_file.m_file != OS_FILE_CLOSED) {
      os_file_close(m_durable_file);
      m_durable_file.m_file = OS_FILE_CLOSED;
    }
  }

  /** Mark archive group inactive.
  A group is marked inactive by archiver background before entering
  into idle state ARCH_STATE_IDLE.
  @param[in]	end_lsn	lsn where redo archiving is stopped */
  void disable(lsn_t end_lsn) {
    m_is_active = false;

    if (end_lsn != LSN_MAX) {
      m_end_lsn = end_lsn;
    }
  }

  /** Attach a client to the archive group.
  @param[in]	is_durable	true, if durable tracking is requested
  @return	number of client references */
  void attach(bool is_durable) {
    ut_ad(mutex_own(m_arch_mutex));
    ++m_num_active;

    if (is_durable) {
      ++m_dur_ref_count;
    } else {
      ++m_ref_count;
    }
  }

  /** Detach a client when archiving is stopped by the client.
  The client still has reference to the group so that the group
  is not destroyed when it retrieves the archived data. The
  reference is removed later by #Arch_Group::release.
  @param[in]	stop_lsn	archive stop lsn for client
  @param[in]	stop_pos	archive stop position for client. Used only by
  the page_archiver.
  @return number of active clients */
  uint detach(lsn_t stop_lsn, Arch_Page_Pos *stop_pos) {
    ut_ad(m_num_active > 0);
    ut_ad(mutex_own(m_arch_mutex));
    --m_num_active;

    if (m_num_active == 0) {
      m_end_lsn = stop_lsn;
      if (stop_pos != nullptr) {
        m_stop_pos = *stop_pos;
      }
    }

    return (m_num_active);
  }

  /** Release the archive group from a client.
  Reduce the reference count. When all clients release the group,
  the reference count falls down to zero. The function would then
  return zero and the caller can remove the group.
  @param[in]	is_durable	the client needs durable archiving */
  void release(bool is_durable) {
    ut_ad(mutex_own(m_arch_mutex));
    ut_a(!is_durable);

    ut_ad(m_ref_count > 0);
    --m_ref_count;
  }

  /** Construct file name for the active file which indicates whether a group
  is active or not.
  @note Used only by the page archiver.
  @return error code. */
  dberr_t build_active_file_name();

  /** Construct file name for the durable file which indicates whether a group
  was made durable or not.
  @note Used only by the page archiver.
  @return error code. */
  dberr_t build_durable_file_name();

  /** Mark the group active by creating a file in the respective group
  directory. This is required at the time of recovery to know whether a group
  was active or not in case of a crash.
  @note Used only by the page archiver.
  @return error code. */
  int mark_active();

  /** Mark the group durable by creating a file in the respective group
  directory. This is required at the time of recovery to differentiate durable
  group from group left over by crash during clone operation.
  @note Used only by the page archiver.
  @return error code. */
  int mark_durable();

  /** Mark the group inactive by deleting the 'active' file. This is required
  at the time of crash recovery to know whether a group was active or not in
  case of a crash.
  @note Used only by the page archiver.
  @return error code */
  int mark_inactive();

  /** Check if archiving is going on for this group
  @return true, if the group is active */
  bool is_active() const { return (m_is_active); }

  /** Write the header (RESET page) to an archived file.
  @note Used only by the Page Archiver and not by the Redo Log Archiver.
  @param[in]	from_buffer	buffer to copy data
  @param[in]	length		size of data to copy in bytes
  @note Used only by the Page Archiver.
  @return error code */
  dberr_t write_file_header(byte *from_buffer, uint length);

  /** Write to the doublewrite buffer before writing archived data to a file.
  The source is either a file context or buffer. Caller must ensure that data
  is in single file in source file context.
  @param[in]	from_file	file context to copy data from
  @param[in]	from_buffer	buffer to copy data or NULL
  @param[in]	write_size	size of data to write in bytes
  @param[in]	offset		offset from where to write
  @note Used only by the Page Archiver.
  @return error code */
  static dberr_t write_to_doublewrite_file(Arch_File_Ctx *from_file,
                                           byte *from_buffer, uint write_size,
                                           Arch_Page_Dblwr_Offset offset);

  /** Archive data to one or more files.
  The source is either a file context or buffer. Caller must ensure that data
  is in single file in source file context.
  @param[in]	from_file	file context to copy data from
  @param[in]	from_buffer	buffer to copy data or NULL
  @param[in]	length		size of data to copy in bytes
  @param[in]	partial_write	true if the operation is part of partial flush
  @param[in]	do_persist	doublewrite to ensure persistence
  @return error code */
  dberr_t write_to_file(Arch_File_Ctx *from_file, byte *from_buffer,
                        uint length, bool partial_write, bool do_persist);

  /** Find the appropriate reset LSN that is less than or equal to the
  given lsn and fetch the reset point.
  @param[in]	check_lsn	LSN to be searched against
  @param[out]	reset_point	reset position of the fetched reset point
  @return true if the search was successful. */
  bool find_reset_point(lsn_t check_lsn, Arch_Point &reset_point) {
    return (m_file_ctx.find_reset_point(check_lsn, reset_point));
  }

  /** Find the first stop LSN that is greater than the given LSN and fetch
  the stop point.
  @param[in]	check_lsn	LSN to be searched against
  @param[out]	stop_point	stop point
  @param[in]	write_pos	latest write_pos
  @return true if the search was successful. */
  bool find_stop_point(lsn_t check_lsn, Arch_Point &stop_point,
                       Arch_Page_Pos write_pos) {
    ut_ad(validate_info_in_files());
    Arch_Page_Pos last_pos = is_active() ? write_pos : m_stop_pos;
    return (m_file_ctx.find_stop_point(this, check_lsn, stop_point, last_pos));
  }

#ifdef UNIV_DEBUG
  /** Adjust end LSN to end of file. This is used in debug
  mode to test the case when LSN is at file boundary.
  @param[in,out]        stop_lsn        stop lsn for client
  @param[out]   blk_len         last block length */
  void adjust_end_lsn(lsn_t &stop_lsn, uint32_t &blk_len);

  /** Adjust redo copy length to end of file. This is used
  in debug mode to archive only till end of file.
  @param[in,out]        length  data to copy in bytes */
  void adjust_copy_length(uint32_t &length);

  /** Check if the information maintained in the memory is the same
  as the information maintained in the files.
  @return true if both sets of information are the same */
  bool validate_info_in_files();
#endif /* UNIV_DEBUG */

  /** Get the total number of archived files belonging to this group.
  @return number of archived files */
  uint get_file_count() const { return (m_file_ctx.get_count()); }

  /** Check if any client (durable or not) is attached to the archiver.
  @return true if any client is attached, else false */
  bool is_referenced() const {
    return (m_ref_count > 0) || (m_dur_ref_count > 0);
  }

  /** Check if any client requiring durable archiving is active.
  @return true if any durable client is still attached, else false */
  bool is_durable_client_active() const {
    return (m_num_active != m_ref_count);
  }

  /** Check if any client requires durable archiving.
  @return true if there is at least 1 client that requires durable archiving*/
  bool is_durable() const { return (m_dur_ref_count > 0); }

  /** Attach system client to the archiver during recovery if any group was
  active at the time of crash. */
  void attach_during_recovery() { ++m_dur_ref_count; }

  /** Purge archived files until the specified purge LSN.
  @param[in]	purge_lsn	LSN until which archived files needs to be
  purged
  @param[out]	purged_lsn	LSN until which purging is successfule;
  LSN_MAX if there was no purging done
  @return error code */
  uint purge(lsn_t purge_lsn, lsn_t &purged_lsn);

  /** Operations to be done at the time of shutdown. */
  static void shutdown() { s_dblwr_file_ctx.close(); }

  /** Update the reset information in the in-memory structure that we maintain
  for faster access.
  @param[in]	lsn     lsn at the time of reset
  @param[in]	pos     pos at the time of reset
  @retval true if the reset point was saved
  @retval false if the reset point wasn't saved because it was already saved */
  void save_reset_point_in_mem(lsn_t lsn, Arch_Page_Pos pos) {
    m_file_ctx.save_reset_point_in_mem(lsn, pos);
  }

  /** Update stop lsn of a file in the group.
  @param[in]	pos		stop position
  @param[in]	stop_lsn	stop point */
  void update_stop_point(Arch_Page_Pos pos, lsn_t stop_lsn) {
    m_file_ctx.update_stop_point(Arch_Block::get_file_index(pos.m_block_num),
                                 stop_lsn);
  }

  /** Recover the information belonging to this group from the archived files.
  @param[in,out]	group_info	structure containing information of a
  group obtained during recovery by scanning files
  @param[in,out]	new_empty_file	true if there is/was an empty archived
  file
  @param[in]		dblwr_ctx	file context related to doublewrite
  buffer
  @param[out]		write_pos	latest write position at the time of
  crash /shutdown that needs to be filled
  @param[out]		reset_pos   latest reset position at the time crash
  /shutdown that needs to be filled
  @return error code */
  dberr_t recover(Arch_Recv_Group_Info *group_info, bool &new_empty_file,
                  Arch_Dblwr_Ctx *dblwr_ctx, Arch_Page_Pos &write_pos,
                  Arch_Page_Pos &reset_pos);

  /** Reads the latest data block and reset block.
  This would be required in case of active group to start page archiving after
  recovery, and in case of inactive group to fetch stop lsn. So we perform this
  operation regardless of whether it's an active or inactive group.
  @param[in]	buf	buffer to read the blocks into
  @param[in]	offset	offset from where to read
  @param[in]	type	block type
  @return error code */
  dberr_t recovery_read_latest_blocks(byte *buf, uint64_t offset,
                                      Arch_Blk_Type type);

  /** Fetch the last reset file and last stop point info during recovery
  @param[out]   reset_file  last reset file to be updated
  @param[out]   stop_lsn    last stop lsn to be updated */
  void recovery_fetch_info(Arch_Reset_File &reset_file, lsn_t &stop_lsn) {
    m_file_ctx.recovery_fetch_info(reset_file, stop_lsn);
  }

#ifdef UNIV_DEBUG
  /** Print recovery related data.
  @param[in]	file_start_index	file index from where to begin */
  void recovery_reset_print(uint file_start_index) {
    DBUG_PRINT("page_archiver", ("Group : %" PRIu64 "", m_begin_lsn));
    m_file_ctx.recovery_reset_print(file_start_index);
    DBUG_PRINT("page_archiver", ("End lsn: %" PRIu64 "", m_end_lsn));
  }
#endif

  /** Parse block for block info (header/data).
  @param[in]	cur_pos		position to read
  @param[in,out]	buff	buffer into which to write the parsed data
  @param[in]	buff_len	length of the buffer
  @return error code */
  int read_data(Arch_Page_Pos cur_pos, byte *buff, uint buff_len);

  /** Get archived file name at specific index in this group.
  Caller would use it to open and copy data from archived files.
  @param[in]	idx		file index in the group
  @param[out]	name_buf	file name and path. Caller must
                                  allocate the buffer.
  @param[in]	buf_len		allocated buffer length */
  void get_file_name(uint idx, char *name_buf, uint buf_len) {
    ut_ad(name_buf != nullptr);

    /* Build name from the file context. */
    m_file_ctx.build_name(idx, m_begin_lsn, name_buf, buf_len);
  }

  /** Get file size for this group.
  Fixed size files are used for archiving data in a group.
  @return file size in bytes */
  uint64_t get_file_size() const { return (m_file_ctx.get_size()); }

  /** Get start LSN for this group
  @return start LSN */
  lsn_t get_begin_lsn() const { return (m_begin_lsn); }

  /** @return stop LSN for this group */
  lsn_t get_end_lsn() const { return (m_end_lsn); }

  /** @return stop block position of the group. */
  Arch_Page_Pos get_stop_pos() const { return (m_stop_pos); }

  /** Fetch the status of the page tracking system.
  @param[out]	status	vector of a pair of (ID, bool) where ID is the
  start/stop point and bool is true if the ID is a start point else false */
  void get_status(std::vector<std::pair<lsn_t, bool>> &status) {
    m_file_ctx.get_status(status);

    if (!is_active()) {
      status.push_back(std::make_pair(m_end_lsn, false));
    }
  }

  /** Disable copy construction */
  Arch_Group(Arch_Group const &) = delete;

  /** Disable assignment */
  Arch_Group &operator=(Arch_Group const &) = delete;

 private:
  /** Get page IDs from archived file
  @param[in]	read_pos	position to read from
  @param[in]	read_len	length of data to read
  @param[in]	read_buff	buffer to read page IDs
  @return error code */
  int read_from_file(Arch_Page_Pos *read_pos, uint read_len, byte *read_buff);

  /** Get the directory name for this archive group.
  It is used for cleaning up the archive directory.
  @param[out]	name_buf	directory name and path. Caller must
                                  allocate the buffer.
  @param[in]	buf_len		buffer length */
  void get_dir_name(char *name_buf, uint buf_len) {
    m_file_ctx.build_dir_name(m_begin_lsn, name_buf, buf_len);
  }

  /** Check and replace blocks in archived files belonging to a group
  from the doublewrite buffer if required.
  @param[in]	dblwr_ctx	Doublewrite context which has the doublewrite
  buffer blocks
  @return error code */
  dberr_t recovery_replace_pages_from_dblwr(Arch_Dblwr_Ctx *dblwr_ctx);

  /** Delete the last file if there are no blocks flushed to it.
  @param[out]	num_files	number of files present in the group
  @param[in]	start_index	file index from where the files are present
  If this is not 0 then the files with file index less that this might have
  been purged.
  @param[in]	durable		true if the group is durable
  @param[out]	empty_file	true if there is/was an empty archived file
  @return error code. */
  dberr_t recovery_cleanup_if_required(uint &num_files, uint start_index,
                                       bool durable, bool &empty_file);

  /** Start parsing the archive file for archive group information.
  @param[out]		write_pos	latest write position at the time of
  crash /shutdown that needs to be filled
  @param[out]		reset_pos   latest reset position at the time crash
  /shutdown that needs to be filled
  @param[in]	start_index	file index from where the files are present
  If this is not 0 then the files with file index less that this might have
  been purged.
  @return error code */
  dberr_t recovery_parse(Arch_Page_Pos &write_pos, Arch_Page_Pos &reset_pos,
                         size_t start_index);

  /** Open the file which was open at the time of a crash, during crash
  recovery, and set the file offset to the last written offset.
  @param[in]	write_pos	block position from where page IDs will be
  tracked
  @param[in]	empty_file	true if an empty archived file was present at
  the time of crash. We delete this file as part of crash recovery process so
  this needs to be handled here.
  @return error code. */
  dberr_t open_file_during_recovery(Arch_Page_Pos write_pos, bool empty_file);

 private:
  /** If the group is active */
  bool m_is_active{true};

  /** To know which group was active at the time of a crash/shutdown during
  recovery we create an empty file in the group directory. This holds the name
  of the file. */
  char *m_active_file_name{nullptr};

  /** File descriptor for a file required to indicate that the group was
  active at the time of crash during recovery . */
  pfs_os_file_t m_active_file;

  /** File name for the durable file which indicates whether a group was made
  durable or not. Required to differentiate durable group from group left over
  by crash during clone operation. */
  char *m_durable_file_name{nullptr};

  /** File descriptor for a file to indicate that the group was made durable or
  not. Required to differentiate durable group from group left over by crash
  during clone operation. */
  pfs_os_file_t m_durable_file;

  /** Number of clients referencing the group */
  uint m_ref_count{};

  /** Number of clients referencing for durable archiving */
  uint m_dur_ref_count{};

  /** Number of clients for which archiving is in progress */
  uint m_num_active{};

  /** Start LSN for the archive group */
  lsn_t m_begin_lsn{LSN_MAX};

  /** End lsn for this archive group */
  lsn_t m_end_lsn{LSN_MAX};

  /** Stop position of the group, if it's not active. */
  Arch_Page_Pos m_stop_pos{};

  /** Header length for the archived files */
  uint m_header_len{};

  /** Archive file context */
  Arch_File_Ctx m_file_ctx;

  /** Doublewrite buffer file context.
  Note - Used only in the case of page archiver. */
  static Arch_File_Ctx s_dblwr_file_ctx;

#ifdef UNIV_DEBUG
  /** Mutex protecting concurrent operations by multiple clients.
  This is either the redo log or page archive system mutex. Currently
  used for assert checks. */
  ib_mutex_t *m_arch_mutex;
#endif /* UNIV_DEBUG */
};
