#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_recv/recv_dblwr_t.h>
#include <innodb/buf_page/buf_flush_t.h>
#include <innodb/univ/page_no_t.h>
#include <innodb/log_recv/MetadataRecover.h>
#include <innodb/log_recv/recv_addr_t.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/sync_mutex/ib_mutex_t.h>

#include <set>
#include <unordered_map>

/** Recovery system data structure */
struct recv_sys_t {
  using Pages =
      std::unordered_map<page_no_t, recv_addr_t *, std::hash<page_no_t>,
                         std::equal_to<page_no_t>>;

  /** Every space has its own heap and pages that belong to it. */
  struct Space {
    /** Constructor
    @param[in,out]	heap	Heap to use for the log records. */
    explicit Space(mem_heap_t *heap) : m_heap(heap), m_pages() {}

    /** Default constructor */
    Space() : m_heap(), m_pages() {}

    /** Memory heap of log records and file addresses */
    mem_heap_t *m_heap;

    /** Pages that need to be recovered */
    Pages m_pages;
  };

  using Missing_Ids = std::set<space_id_t>;

  using Spaces = std::unordered_map<space_id_t, Space, std::hash<space_id_t>,
                                    std::equal_to<space_id_t>>;

  /* Recovery encryption information */
  struct Encryption_Key {
    /** Tablespace ID */
    space_id_t space_id;

    /** Encryption key */
    byte *ptr;

    /** Encryption IV */
    byte *iv;
  };

  using Encryption_Keys = std::vector<Encryption_Key>;

#ifndef UNIV_HOTBACKUP

  /*!< mutex protecting the fields apply_log_recs, n_addrs, and the
  state field in each recv_addr struct */
  ib_mutex_t mutex;

  /** mutex coordinating flushing between recv_writer_thread and
  the recovery thread. */
  ib_mutex_t writer_mutex;

  /** event to activate page cleaner threads */
  os_event_t flush_start;

  /** event to signal that the page cleaner has finished the request */
  os_event_t flush_end;

  /** type of the flush request. BUF_FLUSH_LRU: flush end of LRU,
  keeping free blocks.  BUF_FLUSH_LIST: flush all of blocks. */
  buf_flush_t flush_type;

#endif /* !UNIV_HOTBACKUP */

  /** This is true when log rec application to pages is allowed;
  this flag tells the i/o-handler if it should do log record
  application */
  bool apply_log_recs;

  /** This is true when a log rec application batch is running */
  bool apply_batch_on;

  /** Possible incomplete last recovered log block */
  byte *last_block;

  /** The nonaligned start address of the preceding buffer */
  byte *last_block_buf_start;

  /** Buffer for parsing log records */
  byte *buf;

  /** Size of the parsing buffer */
  size_t buf_len;

  /** Amount of data in buf */
  ulint len;

  /** This is the lsn from which we were able to start parsing
  log records and adding them to the hash table; zero if a suitable
  start point not found yet */
  lsn_t parse_start_lsn;

  /** Checkpoint lsn that was used during recovery (read from file). */
  lsn_t checkpoint_lsn;

  /** Number of data bytes to ignore until we reach checkpoint_lsn. */
  ulint bytes_to_ignore_before_checkpoint;

  /** The log data has been scanned up to this lsn */
  lsn_t scanned_lsn;

  /** The log data has been scanned up to this checkpoint
  number (lowest 4 bytes) */
  ulint scanned_checkpoint_no;

  /** Start offset of non-parsed log records in buf */
  ulint recovered_offset;

  /** The log records have been parsed up to this lsn */
  lsn_t recovered_lsn;

  /** Set when finding a corrupt log block or record, or there
  is a log parsing buffer overflow */
  bool found_corrupt_log;

  /** Set when an inconsistency with the file system contents
  is detected during log scan or apply */
  bool found_corrupt_fs;

  /** If the recovery is from a cloned database. */
  bool is_cloned_db;

  /** Hash table of pages, indexed by SpaceID. */
  Spaces *spaces;

  /** Number of not processed hashed file addresses in the hash table */
  ulint n_addrs;

  /** Doublewrite buffer state during recovery. */
  recv_dblwr_t dblwr;

  /** We store and merge all table persistent data here during
  scanning redo logs */
  MetadataRecover *metadata_recover;

  /** Encryption Key information per tablespace ID */
  Encryption_Keys *keys;

  /** Tablespace IDs that were ignored during redo log apply. */
  Missing_Ids missing_ids;

  /** Tablespace IDs that were explicitly deleted. */
  Missing_Ids deleted;
};

