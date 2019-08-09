#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/flags.h>
#include <innodb/log_types/lsn_t.h>

/** Prefix for name of log file, e.g. "ib_logfile" */
constexpr const char *const ib_logfile_basename = "ib_logfile";

/* base name length(10) + length for decimal digits(22) */
constexpr uint32_t MAX_LOG_FILE_NAME = 32;

/** Magic value to use instead of log checksums when they are disabled. */
constexpr uint32_t LOG_NO_CHECKSUM_MAGIC = 0xDEADBEEFUL;

/** Absolute margin for the free space in the log, before a new query step
which modifies the database, is started. Expressed in number of pages. */
constexpr uint32_t LOG_CHECKPOINT_EXTRA_FREE = 8;

/** Per thread margin for the free space in the log, before a new query step
which modifies the database, is started. It's multiplied by maximum number
of threads, that can concurrently enter mini transactions. Expressed in
number of pages. */
constexpr uint32_t LOG_CHECKPOINT_FREE_PER_THREAD = 4;

/** Controls asynchronous making of a new checkpoint.
Should be bigger than LOG_POOL_PREFLUSH_RATIO_SYNC. */
constexpr uint32_t LOG_POOL_CHECKPOINT_RATIO_ASYNC = 32;

/** Controls synchronous preflushing of modified buffer pages. */
constexpr uint32_t LOG_POOL_PREFLUSH_RATIO_SYNC = 16;

/** Controls asynchronous preflushing of modified buffer pages.
Should be less than the LOG_POOL_PREFLUSH_RATIO_SYNC. */
constexpr uint32_t LOG_POOL_PREFLUSH_RATIO_ASYNC = 8;

/** The counting of lsn's starts from this value: this must be non-zero. */
constexpr lsn_t LOG_START_LSN = 16 * OS_FILE_LOG_BLOCK_SIZE;

/* Offsets used in a log block header. */

/** Block number which must be > 0 and is allowed to wrap around at 1G.
The highest bit is set to 1, if this is the first block in a call to
fil_io (for possibly many consecutive blocks). */
constexpr uint32_t LOG_BLOCK_HDR_NO = 0;

/** Mask used to get the highest bit in the hdr_no field. */
constexpr uint32_t LOG_BLOCK_FLUSH_BIT_MASK = 0x80000000UL;

/** Maximum allowed block's number (stored in hdr_no). */
constexpr uint32_t LOG_BLOCK_MAX_NO = 0x3FFFFFFFUL + 1;

/** Number of bytes written to this block (also header bytes). */
constexpr uint32_t LOG_BLOCK_HDR_DATA_LEN = 4;

/** Mask used to get the highest bit in the data len field,
this bit is to indicate if this block is encrypted or not. */
constexpr uint32_t LOG_BLOCK_ENCRYPT_BIT_MASK = 0x8000UL;

/** Offset of the first start of mtr log record group in this log block.
0 if none. If the value is the same as LOG_BLOCK_HDR_DATA_LEN, it means
that the first rec group has not yet been concatenated to this log block,
but if it will, it will start at this offset.

An archive recovery can start parsing the log records starting from this
offset in this log block, if value is not 0. */
constexpr uint32_t LOG_BLOCK_FIRST_REC_GROUP = 6;

/** 4 lower bytes of the value of log_sys->next_checkpoint_no when the log
block was last written to: if the block has not yet been written full,
this value is only updated before a log buffer flush. */
constexpr uint32_t LOG_BLOCK_CHECKPOINT_NO = 8;

/** Size of the log block's header in bytes. */
constexpr uint32_t LOG_BLOCK_HDR_SIZE = 12;

/* Offsets used in a log block's footer (refer to the end of the block). */

/** 4 byte checksum of the log block contents. In InnoDB versions < 3.23.52
this did not contain the checksum, but the same value as .._HDR_NO. */
constexpr uint32_t LOG_BLOCK_CHECKSUM = 4;

/** Size of the log block footer (trailer) in bytes. */
constexpr uint32_t LOG_BLOCK_TRL_SIZE = 4;

static_assert(LOG_BLOCK_HDR_SIZE + LOG_BLOCK_TRL_SIZE < OS_FILE_LOG_BLOCK_SIZE,
              "Header + footer cannot be larger than the whole log block.");

/** Size of log block's data fragment (where actual data is stored). */
constexpr uint32_t LOG_BLOCK_DATA_SIZE =
    OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_HDR_SIZE - LOG_BLOCK_TRL_SIZE;

/** Ensure, that 64 bits are enough to represent lsn values, when 63 bits
are used to represent sn values. It is enough to ensure that lsn < 2*sn,
and that is guaranteed if the overhead enumerated in lsn sequence is not
bigger than number of actual data bytes. */
static_assert(LOG_BLOCK_HDR_SIZE + LOG_BLOCK_TRL_SIZE < LOG_BLOCK_DATA_SIZE,
              "Overhead in LSN sequence cannot be bigger than actual data.");



/* Offsets inside the checkpoint pages (redo log format version 1). */

/** Checkpoint number. It's incremented by one for each consecutive checkpoint.
During recovery, all headers are scanned, and one with the maximum checkpoint
number is used for the recovery (checkpoint_lsn from the header is used). */
constexpr uint32_t LOG_CHECKPOINT_NO = 0;

/** Checkpoint lsn. Recovery starts from this lsn and searches for the first
log record group that starts since then. In InnoDB < 8.0, it was exact value
at which the first log record group started. Because of the relaxed order in
flush lists, checkpoint lsn values are not precise anymore (the maximum delay
related to the relaxed order in flush lists, is subtracted from oldest_lsn,
when writing a checkpoint). */
constexpr uint32_t LOG_CHECKPOINT_LSN = 8;

/** Offset within the log files, which corresponds to checkpoint lsn.
Used for calibration of lsn and offset calculations. */
constexpr uint32_t LOG_CHECKPOINT_OFFSET = 16;

/** Size of the log buffer, when the checkpoint write was started.
It seems to be write-only field in InnoDB. Not used by recovery.

@note
Note that when the log buffer is being resized, all the log background threads
are stopped, so there no is concurrent checkpoint write (the log_checkpointer
thread is stopped). */
constexpr uint32_t LOG_CHECKPOINT_LOG_BUF_SIZE = 24;

/** Offsets used in a log file header */

/** Log file header format identifier (32-bit unsigned big-endian integer).
This used to be called LOG_GROUP_ID and always written as 0,
because InnoDB never supported more than one copy of the redo log. */
constexpr uint32_t LOG_HEADER_FORMAT = 0;

/** 4 unused (zero-initialized) bytes. */
constexpr uint32_t LOG_HEADER_PAD1 = 4;

/** LSN of the start of data in this log file (with format version 1 and 2). */
constexpr uint32_t LOG_HEADER_START_LSN = 8;

/** A null-terminated string which will contain either the string 'MEB'
and the MySQL version if the log file was created by mysqlbackup,
or 'MySQL' and the MySQL version that created the redo log file. */
constexpr uint32_t LOG_HEADER_CREATOR = 16;

/** End of the log file creator field. */
constexpr uint32_t LOG_HEADER_CREATOR_END = LOG_HEADER_CREATOR + 32;

/** Contents of the LOG_HEADER_CREATOR field */
#define LOG_HEADER_CREATOR_CURRENT "MySQL " INNODB_VERSION_STR

/** Header is created during DB clone */
#define LOG_HEADER_CREATOR_CLONE "MySQL Clone"

/** First checkpoint field in the log header. We write alternately to
the checkpoint fields when we make new checkpoints. This field is only
defined in the first log file. */
constexpr uint32_t LOG_CHECKPOINT_1 = OS_FILE_LOG_BLOCK_SIZE;

/** Second checkpoint field in the header of the first log file. */
constexpr uint32_t LOG_CHECKPOINT_2 = 3 * OS_FILE_LOG_BLOCK_SIZE;


/** Constants related to server variables (default, min and max values). */

/** Default value of innodb_log_write_max_size (in bytes). */
constexpr ulint INNODB_LOG_WRITE_MAX_SIZE_DEFAULT = 4096;

/** Default value of innodb_log_checkpointer_every (in milliseconds). */
constexpr ulong INNODB_LOG_CHECKPOINT_EVERY_DEFAULT = 1000;  // 1000ms = 1s

/** Default value of innodb_log_writer_spin_delay (in spin rounds).
We measured that 1000 spin round takes 4us. We decided to select 1ms
as the maximum time for busy waiting. Therefore it corresponds to 250k
spin rounds. Note that first wait on event takes 50us-100us (even if 10us
is passed), so it is 5%-10% of the total time that we have already spent
on busy waiting, when we fall back to wait on event. */
constexpr ulong INNODB_LOG_WRITER_SPIN_DELAY_DEFAULT = 250000;

/** Default value of innodb_log_writer_timeout (in microseconds).
Note that it will anyway take at least 50us. */
constexpr ulong INNODB_LOG_WRITER_TIMEOUT_DEFAULT = 10;

/** Default value of innodb_log_spin_cpu_abs_lwm.
Expressed in percent (80 stands for 80%) of a single CPU core. */
constexpr ulong INNODB_LOG_SPIN_CPU_ABS_LWM_DEFAULT = 80;

/** Default value of innodb_log_spin_cpu_pct_hwm.
Expressed in percent (50 stands for 50%) of all CPU cores. */
constexpr uint INNODB_LOG_SPIN_CPU_PCT_HWM_DEFAULT = 50;

/** Default value of innodb_log_wait_for_write_spin_delay (in spin rounds).
Read about INNODB_LOG_WRITER_SPIN_DELAY_DEFAULT.
Number of spin rounds is calculated according to current usage of CPU cores.
If the usage is smaller than lwm percents of single core, then max rounds = 0.
If the usage is smaller than 50% of hwm percents of all cores, then max rounds
is decreasing linearly from 10x innodb_log_writer_spin_delay to 1x (for 50%).
Then in range from 50% of hwm to 100% of hwm, the max rounds stays equal to
the innodb_log_writer_spin_delay, because it doesn't make sense to use too
short waits. Hence this is minimum value for the max rounds when non-zero
value is being used. */
constexpr ulong INNODB_LOG_WAIT_FOR_WRITE_SPIN_DELAY_DEFAULT = 25000;

/** Default value of innodb_log_wait_for_write_timeout (in microseconds). */
constexpr ulong INNODB_LOG_WAIT_FOR_WRITE_TIMEOUT_DEFAULT = 1000;

/** Default value of innodb_log_wait_for_flush_spin_delay (in spin rounds).
Read about INNODB_LOG_WAIT_FOR_WRITE_SPIN_DELAY_DEFAULT. The same mechanism
applies here (to compute max rounds). */
constexpr ulong INNODB_LOG_WAIT_FOR_FLUSH_SPIN_DELAY_DEFAULT = 25000;

/** Default value of innodb_log_wait_for_flush_spin_hwm (in microseconds). */
constexpr ulong INNODB_LOG_WAIT_FOR_FLUSH_SPIN_HWM_DEFAULT = 400;

/** Default value of innodb_log_wait_for_flush_timeout (in microseconds). */
constexpr ulong INNODB_LOG_WAIT_FOR_FLUSH_TIMEOUT_DEFAULT = 1000;

/** Default value of innodb_log_flusher_spin_delay (in spin rounds).
Read about INNODB_LOG_WRITER_SPIN_DELAY_DEFAULT. */
constexpr ulong INNODB_LOG_FLUSHER_SPIN_DELAY_DEFAULT = 250000;

/** Default value of innodb_log_flusher_timeout (in microseconds).
Note that it will anyway take at least 50us. */
constexpr ulong INNODB_LOG_FLUSHER_TIMEOUT_DEFAULT = 10;

/** Default value of innodb_log_write_notifier_spin_delay (in spin rounds). */
constexpr ulong INNODB_LOG_WRITE_NOTIFIER_SPIN_DELAY_DEFAULT = 0;

/** Default value of innodb_log_write_notifier_timeout (in microseconds). */
constexpr ulong INNODB_LOG_WRITE_NOTIFIER_TIMEOUT_DEFAULT = 10;

/** Default value of innodb_log_flush_notifier_spin_delay (in spin rounds). */
constexpr ulong INNODB_LOG_FLUSH_NOTIFIER_SPIN_DELAY_DEFAULT = 0;

/** Default value of innodb_log_flush_notifier_timeout (in microseconds). */
constexpr ulong INNODB_LOG_FLUSH_NOTIFIER_TIMEOUT_DEFAULT = 10;

/** Default value of innodb_log_closer_spin_delay (in spin rounds). */
constexpr ulong INNODB_LOG_CLOSER_SPIN_DELAY_DEFAULT = 0;

/** Default value of innodb_log_closer_timeout (in microseconds). */
constexpr ulong INNODB_LOG_CLOSER_TIMEOUT_DEFAULT = 1000;

/** Default value of innodb_log_buffer_size (in bytes). */
constexpr ulong INNODB_LOG_BUFFER_SIZE_DEFAULT = 16 * 1024 * 1024UL;

/** Minimum allowed value of innodb_log_buffer_size. */
constexpr ulong INNODB_LOG_BUFFER_SIZE_MIN = 256 * 1024UL;

/** Maximum allowed value of innodb_log_buffer_size. */
constexpr ulong INNODB_LOG_BUFFER_SIZE_MAX = ULONG_MAX;

/** Default value of innodb_log_recent_written_size (in bytes). */
constexpr ulong INNODB_LOG_RECENT_WRITTEN_SIZE_DEFAULT = 1024 * 1024;

/** Minimum allowed value of innodb_log_recent_written_size. */
constexpr ulong INNODB_LOG_RECENT_WRITTEN_SIZE_MIN = OS_FILE_LOG_BLOCK_SIZE;

/** Maximum allowed value of innodb_log_recent_written_size. */
constexpr ulong INNODB_LOG_RECENT_WRITTEN_SIZE_MAX = 1024 * 1024 * 1024UL;

/** Default value of innodb_log_recent_closed_size (in bytes). */
constexpr ulong INNODB_LOG_RECENT_CLOSED_SIZE_DEFAULT = 2 * 1024 * 1024;

/** Minimum allowed value of innodb_log_recent_closed_size. */
constexpr ulong INNODB_LOG_RECENT_CLOSED_SIZE_MIN = OS_FILE_LOG_BLOCK_SIZE;

/** Maximum allowed value of innodb_log_recent_closed_size. */
constexpr ulong INNODB_LOG_RECENT_CLOSED_SIZE_MAX = 1024 * 1024 * 1024UL;

/** Default value of innodb_log_events (number of events). */
constexpr ulong INNODB_LOG_EVENTS_DEFAULT = 2048;

/** Minimum allowed value of innodb_log_events. */
constexpr ulong INNODB_LOG_EVENTS_MIN = 1;

/** Maximum allowed value of innodb_log_events. */
constexpr ulong INNODB_LOG_EVENTS_MAX = 1024 * 1024 * 1024UL;

/** Default value of innodb_log_write_ahead_size (in bytes). */
constexpr ulong INNODB_LOG_WRITE_AHEAD_SIZE_DEFAULT = 8192;

/** Minimum allowed value of innodb_log_write_ahead_size. */
constexpr ulong INNODB_LOG_WRITE_AHEAD_SIZE_MIN = OS_FILE_LOG_BLOCK_SIZE;

/** Maximum allowed value of innodb_log_write_ahead_size. */
constexpr ulint INNODB_LOG_WRITE_AHEAD_SIZE_MAX =
    UNIV_PAGE_SIZE_DEF;  // 16kB...

/** Value to which MLOG_TEST records should sum up within a group. */
constexpr int64_t MLOG_TEST_VALUE = 10000;

/** Maximum size of single MLOG_TEST record (in bytes). */
constexpr uint32_t MLOG_TEST_MAX_REC_LEN = 100;

/** Maximum number of MLOG_TEST records in single group of log records. */
constexpr uint32_t MLOG_TEST_GROUP_MAX_REC_N = 100;
