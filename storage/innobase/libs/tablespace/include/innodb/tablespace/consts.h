#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

#include <limits>

/** Maximum number of threads to use for scanning data files. */
const size_t MAX_SCAN_THREADS = 8;

#ifndef UNIV_HOTBACKUP
/** Maximum number of shards supported. */
const size_t MAX_SHARDS = 64;

/** The redo log is in its own shard. */
const size_t REDO_SHARD = MAX_SHARDS - 1;

/** Number of undo shards to reserve. */
const size_t UNDO_SHARDS = 4;

/** The UNDO logs have their own shards (4). */
const size_t UNDO_SHARDS_START = REDO_SHARD - (UNDO_SHARDS + 1);
#else  /* !UNIV_HOTBACKUP */

/** Maximum number of shards supported. */
const size_t MAX_SHARDS = 1;

/** The redo log is in its own shard. */
const size_t REDO_SHARD = 0;

/** The UNDO logs have their own shards (4). */
const size_t UNDO_SHARDS_START = 0;
#endif /* !UNIV_HOTBACKUP */

/** Maximum pages to check for valid space ID during start up. */
const size_t MAX_PAGES_TO_CHECK = 3;

/** Sentinel for empty open slot. */
const size_t EMPTY_OPEN_SLOT = std::numeric_limits<size_t>::max();


/** The first ID of the redo log pseudo-tablespace */
static constexpr space_id_t dict_sys_t_s_log_space_first_id = 0xFFFFFFF0UL;

/** Value of fil_space_t::magic_n */
constexpr size_t FIL_SPACE_MAGIC_N = 89472;

/** Value of fil_node_t::magic_n */
constexpr size_t FIL_NODE_MAGIC_N = 89389;

/** Space id of the transaction system page (the system tablespace) */
static const space_id_t TRX_SYS_SPACE = 0;

/** Initial size of a single-table tablespace in pages */
constexpr size_t FIL_IBD_FILE_INITIAL_SIZE = 7;
constexpr size_t FIL_IBT_FILE_INITIAL_SIZE = 5;

/** An empty tablespace (CREATE TABLESPACE) has minimum
of 4 pages and an empty CREATE TABLE (file_per_table) has 6 pages.
Minimum of these two is 4 */
constexpr size_t FIL_IBD_FILE_INITIAL_SIZE_5_7 = 4;
