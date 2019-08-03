#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/pfs_os_file_t.h>
#include <innodb/disk/page_no_t.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/lst/lst.h>

struct fil_space_t;

/** File node of a tablespace or the log data space */
struct fil_node_t {
  using List_node = UT_LIST_NODE_T(fil_node_t);

  /** tablespace containing this file */
  fil_space_t *space;

  /** file name; protected by Fil_shard::m_mutex and log_sys->mutex. */
  char *name;

  /** whether this file is open. Note: We set the is_open flag after
  we increase the write the MLOG_FILE_OPEN record to redo log. Therefore
  we increment the in_use reference count before setting the OPEN flag. */
  bool is_open;

  /** file handle (valid if is_open) */
  pfs_os_file_t handle;

  /** event that groups and serializes calls to fsync */
  os_event_t sync_event;

  /** whether the file actually is a raw device or disk partition */
  bool is_raw_disk;

  /** size of the file in database pages (0 if not known yet);
  the possible last incomplete megabyte may be ignored
  if space->id == 0 */
  page_no_t size;

  /** Size of the file when last flushed, used to force the flush when file
  grows to keep the filesystem metadata synced when using O_DIRECT_NO_FSYNC */
  page_no_t flush_size;

  /** initial size of the file in database pages;
  FIL_IBD_FILE_INITIAL_SIZE by default */
  page_no_t init_size;

  /** maximum size of the file in database pages */
  page_no_t max_size;

  /** count of pending i/o's; is_open must be true if nonzero */
  size_t n_pending;

  /** count of pending flushes; is_open must be true if nonzero */
  size_t n_pending_flushes;

  /** e.g., when a file is being extended or just opened. */
  size_t in_use;

  /** number of writes to the file since the system was started */
  int64_t modification_counter;

  /** the modification_counter of the latest flush to disk */
  int64_t flush_counter;

  /** link to the fil_system->LRU list (keeping track of open files) */
  List_node LRU;

  /** whether the file system of this file supports PUNCH HOLE */
  bool punch_hole;

  /** block size to use for punching holes */
  size_t block_size;

  /** whether atomic write is enabled for this file */
  bool atomic_write;

  /** FIL_NODE_MAGIC_N */
  size_t magic_n;
};
