#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_get_size.h>
#include <innodb/io/os_file_size_t.h>
#include <innodb/assert/assert.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/log_arch/Arch_Reset.h>
#include <innodb/error/dberr_t.h>
#include <innodb/log_sn/lsn_t.h>
#include <innodb/allocator/ut_free.h>

class Arch_Group;

/** Archiver file context.
Represents a set of fixed size files within a group */
class Arch_File_Ctx {
 public:
  /** Constructor: Initialize members */
  Arch_File_Ctx() { m_file.m_file = OS_FILE_CLOSED; }

  /** Destructor: Close open file and free resources */
  ~Arch_File_Ctx() {
    close();

    if (m_name_buf != nullptr) {
      ut_free(m_name_buf);
    }
  }

  /** Initializes archiver file context.
  @param[in]	path		path to the file
  @param[in]	base_dir	directory name prefix
  @param[in]	base_file	file name prefix
  @param[in]	num_files	initial number of files
  @param[in]	file_size	file size in bytes
  @return error code. */
  dberr_t init(const char *path, const char *base_dir, const char *base_file,
               uint num_files, uint64_t file_size);

  /** Open a file at specific index
  @param[in]	read_only	open in read only mode
  @param[in]	start_lsn	start lsn for the group
  @param[in]	file_index	index of the file within the group which needs
  to be opened
  @param[in]	file_offset	start offset
  @return error code. */
  dberr_t open(bool read_only, lsn_t start_lsn, uint file_index,
               uint64_t file_offset);

  /** Add a new file and open
  @param[in]	start_lsn	start lsn for the group
  @param[in]	file_offset	start offset
  @return error code. */
  dberr_t open_new(lsn_t start_lsn, uint64_t file_offset);

  /** Open next file for read
  @param[in]	start_lsn	start lsn for the group
  @param[in]	file_offset	start offset
  @return error code. */
  dberr_t open_next(lsn_t start_lsn, uint64_t file_offset);

  /** Read data from the current file that is open.
  Caller must ensure that the size is within the limits of current file
  context.
  @param[in,out]	to_buffer	read data into this buffer
  @param[in]		offset		file offset from where to read
  @param[in]		size		size of data to read in bytes
  @return error code */
  dberr_t read(byte *to_buffer, const uint offset, const uint size);

  /** Write data to this file context from the given file offset.
  Data source is another file context or buffer. If buffer is NULL, data is
  copied from input file context. Caller must ensure that the size is within
  the limits of current file for both source and destination file context.
  @param[in]	from_file	file context to copy data from
  @param[in]	from_buffer	buffer to copy data or NULL
  @param[in]	offset		file offset from where to write
  @param[in]	size		size of data to copy in bytes
  @return error code */
  dberr_t write(Arch_File_Ctx *from_file, byte *from_buffer, uint offset,
                uint size);

  /** Write data to this file context from the current offset.
  Data source is another file context or buffer. If buffer is NULL, data is
  copied from input file context. Caller must ensure that the size is within
  the limits of current file for both source and destination file context.
  @param[in]	from_file	file context to copy data from
  @param[in]	from_buffer	buffer to copy data or NULL
  @param[in]	size		size of data to copy in bytes
  @return error code */
  dberr_t write(Arch_File_Ctx *from_file, byte *from_buffer, uint size);

  /** Flush file. */
  void flush() {
    if (m_file.m_file != OS_FILE_CLOSED) {
      os_file_flush(m_file);
    }
  }

  /** Close file, if open */
  void close() {
    if (m_file.m_file != OS_FILE_CLOSED) {
      os_file_close(m_file);
      m_file.m_file = OS_FILE_CLOSED;
    }
  }

  /** Check if file is closed
  @return true, if file is closed */
  bool is_closed() const { return (m_file.m_file == OS_FILE_CLOSED); }

  /** Check how much is left in current file
  @return length left in bytes */
  uint64_t bytes_left() const {
    ut_ad(m_size >= m_offset);
    return (m_size - m_offset);
  }

  /** Construct file name at specific index
  @param[in]	idx	file index
  @param[in]	dir_lsn	lsn of the group
  @param[out]	buffer	file name including path.
                          The buffer is allocated by caller.
  @param[in]	length	buffer length */
  void build_name(uint idx, lsn_t dir_lsn, char *buffer, uint length);

  /** Construct group directory name
  @param[in]	dir_lsn	lsn of the group
  @param[out]	buffer	directory name.
                          The buffer is allocated by caller.
  @param[in]	length	buffer length */
  void build_dir_name(lsn_t dir_lsn, char *buffer, uint length);

  /** Get the logical size of a file.
  @return logical file size. */
  uint64_t get_size() const { return (m_size); }

  /* Fetch offset of the file open in this context.
  @return file offset */
  uint get_offset() const { return (m_offset); }

  /** Get number of files
  @return current file count */
  uint get_count() const { return (m_count); }

  /** Get the physical size of a file that is open in this context.
  @return physical file size */
  uint64_t get_phy_size() const {
    ut_ad(m_name_buf != nullptr);
    os_file_size_t file_size = os_file_get_size(m_name_buf);
    return (file_size.m_total_size);
  }

  /** Update stop lsn of a file in the group.
  @param[in]	file_index	file_index the current write_pos belongs to
  @param[in]	stop_lsn	stop point */
  void update_stop_point(uint file_index, lsn_t stop_lsn);

#ifdef UNIV_DEBUG
  /** Print recovery related data.
  @param[in]	file_start_index	file index from where to begin */
  void recovery_reset_print(uint file_start_index);

  /** Check if the information maintained in the memory is the same
  as the information maintained in the files.
  @return true if both sets of information are the same
  @param[in]	group	group whose file is being validated
  @param[in]	file_index	index of the file which is being validated
  @param[in]	start_lsn
  @param[in,out]	reset_count	count of files which has been validated
  @return true if both the sets of information are the same. */
  bool validate(Arch_Group *group, uint file_index, lsn_t start_lsn,
                uint &reset_count);
#endif

  /** Update the reset information in the in-memory structure that we maintain
  for faster access.
  @param[in]	lsn     lsn at the time of reset
  @param[in]	pos     pos at the time of reset
  @retval true if the reset point was saved
  @retval false if the reset point wasn't saved because it was already saved */
  void save_reset_point_in_mem(lsn_t lsn, Arch_Page_Pos pos);

  /** Find the appropriate reset LSN that is less than or equal to the
  given lsn and fetch the reset point.
  @param[in]	check_lsn	LSN to be searched against
  @param[out]	reset_point	reset position of the fetched reset point
  @return true if the search was successful. */
  bool find_reset_point(lsn_t check_lsn, Arch_Point &reset_point);

  /** Find the first stop LSN that is greater than the given LSN and fetch
  the stop point.
  @param[in]	group		the group whose stop_point we're interested in
  @param[in]	check_lsn	LSN to be searched against
  @param[out]	stop_point	stop point
  @param[in]	last_pos	position of the last block in the group;
  m_write_pos if group is active and m_stop_pos if not
  @return true if the search was successful. */
  bool find_stop_point(Arch_Group *group, lsn_t check_lsn,
                       Arch_Point &stop_point, Arch_Page_Pos last_pos);

  /** Delete a single file belonging to the specified file index.
  @param[in]	file_index	file index of the file which needs to be deleted
  @param[in]	begin_lsn	group's start lsn
  @return true if successful, else false. */
  bool delete_file(uint file_index, lsn_t begin_lsn);

  /** Delete all files for this archive group
  @param[in]	begin_lsn	group's start lsn */
  void delete_files(lsn_t begin_lsn);

  /** Purge archived files until the specified purge LSN.
  @param[in]	begin_lsn	start LSN of the group
  @param[in]	end_lsn	end LSN of the group
  @param[in]    purge_lsn   purge LSN until which files needs to be purged
  @return LSN until which purging was successful
  @retval LSN_MAX if there was no purging done. */
  lsn_t purge(lsn_t begin_lsn, lsn_t end_lsn, lsn_t purge_lsn);

  /** Fetch the last reset file and last stop point info during recovery
  @param[out]	reset_file	last reset file to be updated
  @param[out]	stop_lsn	last stop lsn to be updated */
  void recovery_fetch_info(Arch_Reset_File &reset_file, lsn_t &stop_lsn) {
    if (m_reset.size() != 0) {
      reset_file = m_reset.back();
    }

    stop_lsn = get_last_stop_point();
  }

  /** Fetch the status of the page tracking system.
  @param[out]	status	vector of a pair of (ID, bool) where ID is the
  start/stop point and bool is true if the ID is a start point else false */
  void get_status(std::vector<std::pair<lsn_t, bool>> &status) {
    for (auto reset_file : m_reset) {
      for (auto reset_point : reset_file.m_start_point) {
        status.push_back(std::make_pair(reset_point.lsn, true));
      }
    }
  }

  /** @return the stop_point which was stored last */
  lsn_t get_last_stop_point() const {
    if (m_stop_points.size() == 0) {
      return (LSN_MAX);
    }

    return (m_stop_points.back());
  }

  /** Fetch the reset points pertaining to a file.
  @param[in]   file_index      file index of the file from which reset points
  needs to be fetched
  @param[in,out]	reset_pos	Update the reset_pos while fetching the
  reset points
  @return error code. */
  dberr_t fetch_reset_points(uint file_index, Arch_Page_Pos &reset_pos);

  /** Fetch the stop lsn pertaining to a file.
  @param[in]	last_file	true if the file for which the stop point is
  being fetched for is the last file
  @param[in,out]	write_pos	Update the write_pos while fetching the
  stop points
  @return error code. */
  dberr_t fetch_stop_points(bool last_file, Arch_Page_Pos &write_pos);

 private:
#ifdef UNIV_DEBUG
  /** Check if the reset information maintained in the memory is the same
  as the information maintained in the given file.
  @param[in]	file	file descriptor
  @param[in]	file_index	index of the file
  @param[in,out]	reset_count	number of files processed containing
  reset data
  @return true if both sets of information are the same */
  bool validate_reset_block_in_file(pfs_os_file_t file, uint file_index,
                                    uint &reset_count);

  /** Check if the stop LSN maintained in the memory is the same as the
  information maintained in the files.
  @param[in]	group	group whose file is being validated
  @param[in]	file	file descriptor
  @param[in]	file_index	index of the file for which the validation is
  happening
  @return true if both the sets of information are the same. */
  bool validate_stop_point_in_file(Arch_Group *group, pfs_os_file_t file,
                                   uint file_index);
#endif

  /** Fetch reset lsn of a particular reset point pertaining to a file.
  @param[in]   block_num       block number where the reset occured.
  @return reset lsn */
  lsn_t fetch_reset_lsn(uint64_t block_num);

 private:
  /** File name buffer.
  Used if caller doesn't allocate buffer. */
  char *m_name_buf{nullptr};

  /** File name buffer length */
  uint m_name_len{};

  /** Fixed length part of the file.
  Path ended with directory separator. */
  uint m_base_len{};

  /** Fixed part of the path to file */
  const char *m_path_name{nullptr};

  /** Directory name prefix */
  const char *m_dir_name{nullptr};

  /** File name prefix */
  const char *m_file_name{nullptr};

  /** Current file descriptor */
  pfs_os_file_t m_file;

  /** File index within the archive group */
  uint m_index{};

  /** Current number of files in the archive group */
  uint m_count{};

  /** Current file offset */
  uint64_t m_offset{};

  /** File size limit in bytes */
  uint64_t m_size{};

  /** Queue of file structure holding reset information pertaining to
  their respective files in a group.
  Protected by Arch_Page_Sys::m_mutex and Arch_Page_Sys::m_oper_mutex.
  @note used only by the page archiver */
  Arch_Reset m_reset;

  /** Vector of stop points corresponding to a file.
  Stop point refers to the stop lsn (checkpoint lsn) until which the pages are
  guaranteed to be tracked in a file. Each block in a file maintains this
  information.
  Protected by Arch_Page_Sys::m_oper_mutex.
  @note used only by the page archiver */
  std::vector<lsn_t> m_stop_points;
};
