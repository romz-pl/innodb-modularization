#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/lst/lst.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/page/page_size_t.h>
#include <innodb/sync_mutex/mutex_destroy.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/tablespace/AIO_mode.h>
#include <innodb/tablespace/Char_Ptr_Compare.h>
#include <innodb/tablespace/Char_Ptr_Hash.h>
#include <innodb/tablespace/Fil_iterator.h>
#include <innodb/tablespace/buf_remove_t.h>
#include <innodb/tablespace/fil_load_status.h>
#include <innodb/tablespace/fil_node_t.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/tablespace/page_id_t.h>


#include <unordered_map>

class Fil_shard {
  using File_list = UT_LIST_BASE_NODE_T(fil_node_t);
  using Space_list = UT_LIST_BASE_NODE_T(fil_space_t);
  using Spaces = std::unordered_map<space_id_t, fil_space_t *>;

  using Names = std::unordered_map<const char *, fil_space_t *, Char_Ptr_Hash,
                                   Char_Ptr_Compare>;

 public:
  /** Constructor
  @param[in]	shard_id	Shard ID  */
  explicit Fil_shard(size_t shard_id);

  /** Destructor */
  ~Fil_shard() {
#ifndef UNIV_HOTBACKUP
    mutex_destroy(&m_mutex);
#else
    mutex_free(&m_mutex);
#endif /* !UNIV_HOTBACKUP */

    ut_a(UT_LIST_GET_LEN(m_LRU) == 0);
    ut_a(UT_LIST_GET_LEN(m_unflushed_spaces) == 0);
  }

  /** @return the shard ID */
  size_t id() const { return (m_id); }

  /** Acquire the mutex.
  @param[in]	line	Line number from where it was called */
  void acquire(int line) const {
#ifndef UNIV_HOTBACKUP
    m_mutex.enter(srv_n_spin_wait_rounds, srv_spin_wait_delay, __FILE__, line);
#else
    mutex_enter(&m_mutex);
#endif /* !UNIV_HOTBACKUP */
  }

  /** Release the mutex. */
  void mutex_release() const { mutex_exit(&m_mutex); }

#ifdef UNIV_DEBUG
  /** @return true if the mutex is owned. */
  bool mutex_owned() const { return (mutex_own(&m_mutex)); }
#endif /* UNIV_DEBUG */

    /** Mutex protecting this shard. */

#ifndef UNIV_HOTBACKUP
  mutable ib_mutex_t m_mutex;
#else
  mutable meb::Mutex m_mutex;
#endif /* !UNIV_HOTBACKUP */

  /** Fetch the fil_space_t instance that maps to space_id.
  @param[in]	space_id	Tablespace ID to lookup
  @return tablespace instance or nullptr if not found. */
  fil_space_t *get_space_by_id(space_id_t space_id) const
      MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(m_id == REDO_SHARD || mutex_owned());

    auto it = m_spaces.find(space_id);

    if (it == m_spaces.end()) {
      return (nullptr);
    }

    ut_ad(it->second->magic_n == FIL_SPACE_MAGIC_N);

    return (it->second);
  }

  /** Fetch the fil_space_t instance that maps to the name.
  @param[in]	name		Tablespace name to lookup
  @return tablespace instance or nullptr if not found. */
  fil_space_t *get_space_by_name(const char *name) const
      MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(mutex_owned());

    auto it = m_names.find(name);

    if (it == m_names.end()) {
      return (nullptr);
    }

    ut_ad(it->second->magic_n == FIL_SPACE_MAGIC_N);

    return (it->second);
  }

  /** Tries to close a file in the shard LRU list.
  The caller must hold the Fil_shard::m_mutex.
  @param[in] print_info		if true, prints information
                                  why it cannot close a file
  @return true if success, false if should retry later */
  bool close_files_in_LRU(bool print_info) MY_ATTRIBUTE((warn_unused_result));

  /** Remove the file node from the LRU list.
  @param[in,out]	file		File for the tablespace */
  void remove_from_LRU(fil_node_t *file);

  /** Add the file node to the LRU list if required.
  @param[in,out]	file		File for the tablespace */
  void file_opened(fil_node_t *file);

  /** Open all the system files.
  @param[in]	max_n_open	Max files that can be opened.
  @param[in]	n_open		Current number of open files */
  void open_system_tablespaces(size_t max_n_open, size_t *n_open);

  /** Close a tablespace file.
  @param[in,out]	file		Tablespace file to close
  @param[in]	LRU_close	true if called from LRU close */
  void close_file(fil_node_t *file, bool LRU_close);

  /** Close a tablespace file based on tablespace ID.
  @param[in]	space_id	Tablespace ID
  @return false if space_id was not found. */
  bool close_file(space_id_t space_id);

  /** Prepare to free a file object from a tablespace
  memory cache.
  @param[in,out]	file	Tablespace file
  @param[in]	space	tablespace */
  void file_close_to_free(fil_node_t *file, fil_space_t *space);

  /** Close log files.
  @param[in]	free_all	If set then free all */
  void close_log_files(bool free_all);

  /** Close all open files. */
  void close_all_files();

  /** Detach a space object from the tablespace memory cache.
  Closes the tablespace files but does not delete them.
  There must not be any pending I/O's or flushes on the files.
  @param[in,out]	space		tablespace */
  void space_detach(fil_space_t *space);

  /** Delete the instance that maps to space_id
  @param[in]	space_id	Tablespace ID to delete */
  void space_delete(space_id_t space_id) {
    ut_ad(mutex_owned());

    auto it = m_spaces.find(space_id);

    if (it != m_spaces.end()) {
      m_names.erase(it->second->name);
      m_spaces.erase(it);
    }
  }

  /** Frees a space object from the tablespace memory cache.
  Closes a tablespaces' files but does not delete them.
  There must not be any pending I/O's or flushes on the files.
  @param[in]	space_id	Tablespace ID
  @return fil_space_t instance on success or nullptr */
  fil_space_t *space_free(space_id_t space_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** Map the space ID and name to the tablespace instance.
  @param[in]	space		Tablespace instance */
  void space_add(fil_space_t *space);

  /** Prepare to free a file. Remove from the unflushed list
  if there are no pending flushes.
  @param[in,out]	file		File instance to free */
  void prepare_to_free_file(fil_node_t *file);

  /** If the tablespace is on the unflushed list and there
  are no pending flushes then remove from the unflushed list.
  @param[in,out]	space		Tablespace to remove*/
  void remove_from_unflushed_list(fil_space_t *space);

  /** Updates the data structures when an I/O operation
  finishes. Updates the pending I/O's field in the file
  appropriately.
  @param[in]	file		Tablespace file
  @param[in]	type		Marks the file as modified
                                  if type == WRITE */
  void complete_io(fil_node_t *file, const IORequest &type);

  /** Prepares a file for I/O. Opens the file if it is closed.
  Updates the pending I/O's field in the file and the system
  appropriately. Takes the file off the LRU list if it is in
  the LRU list.
  @param[in]	file		Tablespace file for IO
  @param[in]	extend		true if file is being extended
  @return false if the file can't be opened, otherwise true */
  bool prepare_file_for_io(fil_node_t *file, bool extend)
      MY_ATTRIBUTE((warn_unused_result));

  /** Reserves the mutex and tries to make sure we can
  open at least one file while holding it. This should be called
  before calling prepare_file_for_io(), because that function
  may need to open a file.
  @param[in]	space_id	Tablespace ID
  @param[out]	space		Tablespace instance
  @return true if a slot was reserved. */
  bool mutex_acquire_and_get_space(space_id_t space_id, fil_space_t *&space)
      MY_ATTRIBUTE((warn_unused_result));

  /** Remap the tablespace to the new name.
  @param[in]	space		Tablespace instance with old name
  @param[in]	new_name	New tablespace name */
  void update_space_name_map(fil_space_t *space, const char *new_name);

  /** Flush the redo log writes to disk, possibly cached by the OS. */
  void flush_file_redo();

  /** Collect the tablespace IDs of unflushed tablespaces in space_ids.
  @param[in]	purpose		FIL_TYPE_TABLESPACE or FIL_TYPE_LOG,
                                  can be ORred */
  void flush_file_spaces(uint8_t purpose);

  /** Try to extend a tablespace if it is smaller than the specified size.
  @param[in,out]	space		tablespace
  @param[in]	size		desired size in pages
  @return whether the tablespace is at least as big as requested */
  bool space_extend(fil_space_t *space, page_no_t size)
      MY_ATTRIBUTE((warn_unused_result));

  /** Flushes to disk possible writes cached by the OS. If the space does
  not exist or is being dropped, does not do anything.
  @param[in]	space_id	File space ID (this can be a group of
                                  log files or a tablespace of the
                                  database) */
  void space_flush(space_id_t space_id);

  /** Open a file of a tablespace.
  The caller must own the fil_system mutex.
  @param[in,out]	file		Tablespace file
  @param[in]	extend		true if the file is being extended
  @return false if the file can't be opened, otherwise true */
  bool open_file(fil_node_t *file, bool extend)
      MY_ATTRIBUTE((warn_unused_result));

  /** Checks if all the file nodes in a space are flushed.
  The caller must hold all fil_system mutexes.
  @param[in]	space		Tablespace to check
  @return true if all are flushed */
  bool space_is_flushed(const fil_space_t *space)
      MY_ATTRIBUTE((warn_unused_result));

  /** Open each file of a tablespace if not already open.
  @param[in]	space_id	tablespace identifier
  @retval	true	if all file nodes were opened
  @retval	false	on failure */
  bool space_open(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result));

  /** Opens the files associated with a tablespace and returns a
  pointer to the fil_space_t that is in the memory cache associated
  with a space id.
  @param[in]	space_id	Get the tablespace instance or this ID
  @return file_space_t pointer, nullptr if space not found */
  fil_space_t *space_load(space_id_t space_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** Check pending operations on a tablespace.
  @param[in]	space_id	Tablespace ID
  @param[out]	space		tablespace instance in memory
  @param[out]	path		tablespace path
  @return DB_SUCCESS or DB_TABLESPACE_NOT_FOUND. */
  dberr_t space_check_pending_operations(space_id_t space_id,
                                         fil_space_t *&space, char **path) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Rename a single-table tablespace.
  The tablespace must exist in the memory cache.
  @param[in]	space_id	Tablespace ID
  @param[in]	old_path	Old file name
  @param[in]	new_name	New tablespace  name in the schema/space
  @param[in]	new_path_in	New file name, or nullptr if it
                                  is located in the normal data directory
  @return InnoDB error code */
  dberr_t space_rename(space_id_t space_id, const char *old_path,
                       const char *new_name, const char *new_path_in)
      MY_ATTRIBUTE((warn_unused_result));

  /** Deletes an IBD tablespace, either general or single-table.
  The tablespace must be cached in the memory cache. This will delete the
  datafile, fil_space_t & fil_node_t entries from the file_system_t cache.
  @param[in]	space_id	Tablespace ID
  @param[in]	buf_remove	Specify the action to take on the pages
                                  for this table in the buffer pool.
  @return DB_SUCCESS, DB_TABLESPCE_NOT_FOUND or DB_IO_ERROR */
  dberr_t space_delete(space_id_t space_id, buf_remove_t buf_remove)
      MY_ATTRIBUTE((warn_unused_result));

  /** Truncate the tablespace to needed size.
  @param[in]	space_id	Tablespace ID to truncate
  @param[in]	size_in_pages	Truncate size.
  @return true if truncate was successful. */
  bool space_truncate(space_id_t space_id, page_no_t size_in_pages)
      MY_ATTRIBUTE((warn_unused_result));

  /** Create a space memory object and put it to the fil_system hash
  table. The tablespace name is independent from the tablespace file-name.
  Error messages are issued to the server log.
  @param[in]	name		Tablespace name
  @param[in]	space_id	Tablespace ID
  @param[in]	flags		Tablespace flags
  @param[in]	purpose		Tablespace purpose
  @return pointer to created tablespace
  @retval nullptr on failure (such as when the same tablespace exists) */
  fil_space_t *space_create(const char *name, space_id_t space_id,
                            uint32_t flags, fil_type_t purpose)
      MY_ATTRIBUTE((warn_unused_result));

  /** Adjust temporary auto-generated names created during
  file discovery with correct tablespace names from the DD.
  @param[in,out]	space		Tablespace
  @param[in]	dd_space_name	Tablespace name from the DD
  @return true if the tablespace is a general or undo tablespace. */
  bool adjust_space_name(fil_space_t *space, const char *dd_space_name);

  /** Returns true if a matching tablespace exists in the InnoDB
  tablespace memory cache.
  @param[in]	space_id	Tablespace ID
  @param[in]	name		Tablespace name used in space_create().
  @param[in]	print_err	Print detailed error information to the
                                  error log if a matching tablespace is
                                  not found from memory.
  @param[in]	adjust_space	Whether to adjust space id on mismatch
  @param[in]	heap			Heap memory
  @param[in]	table_id		table id
  @return true if a matching tablespace exists in the memory cache */
  bool space_check_exists(space_id_t space_id, const char *name, bool print_err,
                          bool adjust_space, mem_heap_t *heap,
                          table_id_t table_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** Read or write log file data synchronously.
  @param[in]	type		IO context
  @param[in]	page_id		page id
  @param[in]	page_size	page size
  @param[in]	byte_offset	remainder of offset in bytes; in AIO
                                  this must be divisible by the OS block
                                  size
  @param[in]	len		how many bytes to read or write; this
                                  must not cross a file boundary; in AIO
                                  this must be a block size multiple
  @param[in,out]	buf		buffer where to store read data or
                                  from where to write
  @return error code
  @retval DB_SUCCESS on success */
  dberr_t do_redo_io(const IORequest &type, const page_id_t &page_id,
                     const page_size_t &page_size, ulint byte_offset, ulint len,
                     void *buf) MY_ATTRIBUTE((warn_unused_result));

  /** Read or write data. This operation could be asynchronous (aio).
  @param[in]	type		IO context
  @param[in]	sync		whether synchronous aio is desired
  @param[in]	page_id		page id
  @param[in]	page_size	page size
  @param[in]	byte_offset	remainder of offset in bytes; in AIO
                                  this must be divisible by the OS
                                  block size
  @param[in]	len		how many bytes to read or write;
                                  this must not cross a file boundary;
                                  in AIO this must be a block size
                                  multiple
  @param[in,out]	buf		buffer where to store read data
                                  or from where to write; in AIO
                                  this must be appropriately aligned
  @param[in]	message		message for AIO handler if !sync,
                                  else ignored
  @return error code
  @retval DB_SUCCESS on success
  @retval DB_TABLESPACE_DELETED if the tablespace does not exist */
  dberr_t do_io(const IORequest &type, bool sync, const page_id_t &page_id,
                const page_size_t &page_size, ulint byte_offset, ulint len,
                void *buf, void *message) MY_ATTRIBUTE((warn_unused_result));

  /** Iterate through all persistent tablespace files
  (FIL_TYPE_TABLESPACE) returning the nodes via callback function cbk.
  @param[in]	include_log	include log files, if true
  @param[in]	f		Callback
  @return any error returned by the callback function. */
  dberr_t iterate(bool include_log, Fil_iterator::Function &f)
      MY_ATTRIBUTE((warn_unused_result));

  /** Open an ibd tablespace and add it to the InnoDB data structures.
  This is similar to fil_ibd_open() except that it is used while
  processing the redo and DDL log, so the data dictionary is not
  available and very little validation is done. The tablespace name
  is extracted from the dbname/tablename.ibd portion of the filename,
  which assumes that the file is a file-per-table tablespace. Any name
  will do for now. General tablespace names will be read from the
  dictionary after it has been recovered. The tablespace flags are read
  at this time from the first page of the file in validate_for_recovery().
  @param[in]	space_id	tablespace ID
  @param[in]	path		path/to/databasename/tablename.ibd
  @param[out]	space		the tablespace, or nullptr on error
  @return status of the operation */
  fil_load_status ibd_open_for_recovery(space_id_t space_id,
                                        const std::string &path,
                                        fil_space_t *&space)
      MY_ATTRIBUTE((warn_unused_result));

  /** Attach a file to a tablespace
  @param[in]	name		file name of a file that is not open
  @param[in]	size		file size in entire database blocks
  @param[in,out]	space		tablespace from fil_space_create()
  @param[in]	is_raw		true if this is a raw device
                                  or partition
  @param[in]	punch_hole	true if supported for this file
  @param[in]	atomic_write	true if the file has atomic write
                                  enabled
  @param[in]	max_pages	maximum number of pages in file
  @return pointer to the file name
  @retval nullptr if error */
  fil_node_t *create_node(const char *name, page_no_t size, fil_space_t *space,
                          bool is_raw, bool punch_hole, bool atomic_write,
                          page_no_t max_pages = PAGE_NO_MAX)
      MY_ATTRIBUTE((warn_unused_result));

#ifdef UNIV_DEBUG
  /** Validate a shard. */
  void validate() const;
#endif /* UNIV_DEBUG */

#ifdef UNIV_HOTBACKUP
  /** Extends all tablespaces to the size stored in the space header.
  During the mysqlbackup --apply-log phase we extended the spaces
  on-demand so that log records could be applied, but that may have
  left spaces still too small compared to the size stored in the space
  header. */
  void meb_extend_tablespaces_to_stored_len();
#endif /* UNIV_HOTBACKUP */

  /** Free a tablespace object on which fil_space_detach() was invoked.
  There must not be any pending i/o's or flushes on the files.
  @param[in,out]	space		tablespace */
  static void space_free_low(fil_space_t *&space);

  /** Wait for an empty slot to reserve for opening a file.
  @return true on success. */
  static bool reserve_open_slot(size_t shard_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** Release the slot reserved for opening a file.
  @param[in]	shard_id	ID of shard relasing the slot */
  static void release_open_slot(size_t shard_id);

  /** We are going to do a rename file and want to stop new I/O
  for a while.
  @param[in]	space		Tablespace for which we want to
                                  wait for IO to stop */
  static void wait_for_io_to_stop(const fil_space_t *space);

 private:
  /** We keep log files and system tablespace files always open; this is
  important in preventing deadlocks in this module, as a page read
  completion often performs another read from the insert buffer. The
  insert buffer is in tablespace TRX_SYS_SPACE, and we cannot end up
  waiting in this function.
  @param[in]	space_id	Tablespace ID to look up
  @return tablespace instance */
  fil_space_t *get_reserved_space(space_id_t space_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** Prepare for truncating a single-table tablespace.
  1) Check pending operations on a tablespace;
  2) Remove all insert buffer entries for the tablespace;
  @param[in]	space_id	Tablespace ID
  @return DB_SUCCESS or error */
  dberr_t space_prepare_for_truncate(space_id_t space_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** Note that a write IO has completed.
  @param[in,out]	file		File on which a write was
                                  completed */
  void write_completed(fil_node_t *file);

  /** If the tablespace is not on the unflushed list, add it.
  @param[in,out]	space		Tablespace to add */
  void add_to_unflushed_list(fil_space_t *space);

  /** Check for pending operations.
  @param[in]	space	tablespace
  @param[in]	count	number of attempts so far
  @return 0 if no pending operations else count + 1. */
  ulint space_check_pending_operations(fil_space_t *space, ulint count) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Check for pending IO.
  @param[in]	space		Tablespace to check
  @param[in]	file		File in space list
  @param[in]	count		number of attempts so far
  @return 0 if no pending else count + 1. */
  ulint check_pending_io(const fil_space_t *space, const fil_node_t &file,
                         ulint count) const MY_ATTRIBUTE((warn_unused_result));

  /** Flushes to disk possible writes cached by the OS. */
  void redo_space_flush();

  /** First we open the file in the normal mode, no async I/O here, for
  simplicity. Then do some checks, and close the file again.  NOTE that we
  could not use the simple file read function os_file_read() in Windows
  to read from a file opened for async I/O!
  @param[in,out]	file		Get the size of this file
  @param[in]	read_only_mode	true if read only mode set
  @return DB_SUCCESS or error */
  dberr_t get_file_size(fil_node_t *file, bool read_only_mode)
      MY_ATTRIBUTE((warn_unused_result));

  /** Get the AIO mode.
  @param[in]	req_type	IO request type
  @param[in]	sync		true if Synchronous IO
  return the AIO mode */
  static AIO_mode get_AIO_mode(const IORequest &req_type, bool sync)
      MY_ATTRIBUTE((warn_unused_result));

  /** Get the file name for IO and the local offset within that file.
  @param[in]	req_type	IO context
  @param[in,out]	space		Tablespace for IO
  @param[in,out]	page_no		The relative page number in the file
  @param[out]	file		File node
  @return DB_SUCCESS or error code */
  static dberr_t get_file_for_io(const IORequest &req_type, fil_space_t *space,
                                 page_no_t *page_no, fil_node_t *&file)
      MY_ATTRIBUTE((warn_unused_result));

 private:
  /** Fil_shard ID */

  const size_t m_id;

  /** Tablespace instances hashed on the space id */

  Spaces m_spaces;

  /** Tablespace instances hashed on the space name */

  Names m_names;

  /** Base node for the LRU list of the most recently used open
  files with no pending I/O's; if we start an I/O on the file,
  we first remove it from this list, and return it to the start
  of the list when the I/O ends; log files and the system
  tablespace are not put to this list: they are opened after
  the startup, and kept open until shutdown */

  File_list m_LRU;

  /** Base node for the list of those tablespaces whose files
  contain unflushed writes; those spaces have at least one file
  where modification_counter > flush_counter */

  Space_list m_unflushed_spaces;

  /** When we write to a file we increment this by one */

  int64_t m_modification_counter;

  /** Number of files currently open */

  static std::atomic_size_t s_n_open;

  /** ID of shard that has reserved the open slot. */

  static std::atomic_size_t s_open_slot;

  // Disable copying
  Fil_shard(Fil_shard &&) = delete;
  Fil_shard(const Fil_shard &) = delete;
  Fil_shard &operator=(const Fil_shard &) = delete;

  friend class Fil_system;
};
