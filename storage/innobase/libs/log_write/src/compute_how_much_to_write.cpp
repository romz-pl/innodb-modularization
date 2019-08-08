#include <innodb/log_write/compute_how_much_to_write.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_redo/flags.h>
#include <innodb/log_redo/srv_log_write_ahead_size.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/compute_next_write_ahead_end.h>
#include <innodb/log_write/current_file_has_space.h>
#include <innodb/log_write/current_write_ahead_enough.h>
#include <innodb/log_write/write_ahead_enough.h>
#include <innodb/monitor/MONITOR_INC.h>

namespace Log_files_write_impl {

size_t compute_how_much_to_write(const log_t &log,
                                               uint64_t real_offset,
                                               size_t buffer_size,
                                               bool &write_from_log_buffer) {
  size_t write_size;

  /* First we ensure, that we will write within single log file.
  If we had more to write and cannot fit the current log file,
  we first write what fits, then stops and returns to the main
  loop of the log writer thread. Then, the log writer will update
  maximum lsn up to which, it has data ready in the log buffer,
  and request next write operation according to its strategy. */
  if (!current_file_has_space(log, real_offset, buffer_size)) {
    /* The end of write would not fit the current log file. */

    /* But the beginning is guaranteed to fit or to be placed
    at the first byte of the next file. */
    ut_a(current_file_has_space(log, real_offset, 0));

    if (!current_file_has_space(log, real_offset, 1)) {
      /* The beginning of write is at the first byte
      of the next log file. Flush header of the next
      log file, advance current log file to the next,
      stop and return to the main loop of log writer. */
      write_from_log_buffer = false;
      return (0);

    } else {
      /* We write across at least two consecutive log files.
      Limit current write to the first one and then retry for
      next_file. */

      /* If the condition for real_offset + buffer_size holds,
      then the expression below is < buffer_size, which is
      size_t, so the typecast is ok. */
      write_size =
          static_cast<size_t>(log.current_file_end_offset - real_offset);

      ut_a(write_size <= buffer_size);
      ut_a(write_size % OS_FILE_LOG_BLOCK_SIZE == 0);
    }

  } else {
    write_size = buffer_size;

    ut_a(write_size % OS_FILE_LOG_BLOCK_SIZE >= LOG_BLOCK_HDR_SIZE ||
         write_size % OS_FILE_LOG_BLOCK_SIZE == 0);

    ut_a(write_size % OS_FILE_LOG_BLOCK_SIZE <
         OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);
  }

  /* Now, we know we can write write_size bytes from the buffer,
  and we will do the write within single log file - current one. */

  ut_a(write_size > 0);
  ut_a(real_offset >= log.current_file_real_offset);
  ut_a(real_offset + write_size <= log.current_file_end_offset);
  ut_a(log.current_file_real_offset / log.file_size + 1 ==
       log.current_file_end_offset / log.file_size);

  /* We are interested in writing from log buffer only,
  if we had at least one completed block for write.
  Still we might decide not to write from the log buffer,
  because write-ahead is needed. In such case we could write
  together with the last incomplete block after copying. */
  write_from_log_buffer = write_size >= OS_FILE_LOG_BLOCK_SIZE;

  if (write_from_log_buffer) {
    MONITOR_INC(MONITOR_LOG_FULL_BLOCK_WRITES);
  } else {
    MONITOR_INC(MONITOR_LOG_PARTIAL_BLOCK_WRITES);
  }

  /* Check how much we have written ahead to avoid read-on-write. */

  if (!current_write_ahead_enough(log, real_offset, write_size)) {
    if (!current_write_ahead_enough(log, real_offset, 1)) {
      /* Current write-ahead region has no space at all. */

      const auto next_wa = compute_next_write_ahead_end(real_offset);

      if (!write_ahead_enough(next_wa, real_offset, write_size)) {
        /* ... and also the next write-ahead is too small.
        Therefore we have more data to write than size of
        the write-ahead. We write from the log buffer,
        skipping last fragment for which the write ahead
        is required. */

        ut_a(write_from_log_buffer);

        write_size = next_wa - real_offset;

        ut_a((real_offset + write_size) % srv_log_write_ahead_size == 0);

        ut_a(write_size % OS_FILE_LOG_BLOCK_SIZE == 0);

      } else {
        /* We copy data to write_ahead buffer,
        and write from there doing write-ahead
        of the bigger region in the same time. */
        write_from_log_buffer = false;
      }

    } else {
      /* We limit write up to the end of region
      we have written ahead already. */
      write_size =
          static_cast<size_t>(log.write_ahead_end_offset - real_offset);

      ut_a(write_size >= OS_FILE_LOG_BLOCK_SIZE);
      ut_a(write_size % OS_FILE_LOG_BLOCK_SIZE == 0);
    }

  } else {
    if (write_from_log_buffer) {
      write_size = ut_uint64_align_down(write_size, OS_FILE_LOG_BLOCK_SIZE);
    }
  }

  return (write_size);
}



}
