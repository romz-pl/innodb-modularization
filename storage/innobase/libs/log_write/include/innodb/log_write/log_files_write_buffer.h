#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

struct log_t;

/** Writes fragment of log buffer to log files. The first write to the first
log block in a new log file, flushes header of the file. It stops after doing
single fil_io operation. The reason is that it might make sense to advance
lsn up to which we have ready data in log buffer for write, after time
consuming operation, such as fil_io. The log.write_lsn is advanced.
@param[in]  log            redo log
@param[in]  buffer         the beginning of first log block to write
@param[in]  buffer_size    number of bytes to write since 'buffer'
@param[in]  start_lsn	lsn  corresponding to first block start */
void log_files_write_buffer(log_t &log, byte *buffer, size_t buffer_size,
                                   lsn_t start_lsn);
