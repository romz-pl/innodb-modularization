#include <innodb/log_redo/log_buffer_write.h>

#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_block/log_block_set_first_rec_group.h>
#include <innodb/log_redo/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_sn/log_lsn_validate.h>
#include <innodb/log_redo/log_translate_lsn_to_sn.h>
#include <innodb/log_redo/log_translate_lsn_to_sn.h>
#include <innodb/log_types/log_t.h>

lsn_t log_buffer_write(log_t &log, const Log_handle &handle, const byte *str,
                       size_t str_len, lsn_t start_lsn) {
  ut_ad(log.sn_lock.s_own(handle.lock_no));

  ut_a(log.buf != nullptr);
  ut_a(log.buf_size > 0);
  ut_a(log.buf_size % OS_FILE_LOG_BLOCK_SIZE == 0);
  ut_a(str != nullptr);
  ut_a(str_len > 0);

  /* We should first resize the log buffer, if str_len is that big. */
  ut_a(str_len < log.buf_size_sn.load());

  /* The start_lsn points a data byte (not a header of log block). */
  ut_a(log_lsn_validate(start_lsn));

  /* We neither write with holes, nor overwrite any fragments of data. */
  ut_ad(log.write_lsn.load() <= start_lsn);
  ut_ad(log_buffer_ready_for_write_lsn(log) <= start_lsn);

  /* That's only used in the assertion at the very end. */
  const lsn_t end_sn = log_translate_lsn_to_sn(start_lsn) + str_len;

  /* A guard used to detect when we should wrap (to avoid overflowing
  outside the log buffer). */
  byte *buf_end = log.buf + log.buf_size;

  /* Pointer to next data byte to set within the log buffer. */
  byte *ptr = log.buf + (start_lsn % log.buf_size);

  /* Lsn value for the next byte to copy. */
  lsn_t lsn = start_lsn;

  /* Copy log records to the reserved space in the log buffer.
  Decrease number of bytes to copy (str_len) after some are
  copied. Proceed until number of bytes to copy reaches zero. */
  while (true) {
    /* Calculate offset from the beginning of log block. */
    const auto offset = lsn % OS_FILE_LOG_BLOCK_SIZE;

    ut_a(offset >= LOG_BLOCK_HDR_SIZE);
    ut_a(offset < OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);

    /* Calculate how many free data bytes are available
    within current log block. */
    const auto left = OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE - offset;

    ut_a(left > 0);
    ut_a(left < OS_FILE_LOG_BLOCK_SIZE);

    size_t len, lsn_diff;

    if (left > str_len) {
      /* There are enough free bytes to finish copying
      the remaining part, leaving at least single free
      data byte in the log block. */

      len = str_len;

      lsn_diff = str_len;

    } else {
      /* We have more to copy than the current log block
      has remaining data bytes, or exactly the same.

      In both cases, next lsn value will belong to the
      next log block. Copy data up to the end of the
      current log block and start a next iteration if
      there is more to copy. */

      len = left;

      lsn_diff = left + LOG_BLOCK_TRL_SIZE + LOG_BLOCK_HDR_SIZE;
    }

    ut_a(len > 0);
    ut_a(ptr + len <= buf_end);

    LOG_SYNC_POINT("log_buffer_write_before_memcpy");

    /* This is the critical memcpy operation, which copies data
    from internal mtr's buffer to the shared log buffer. */
    std::memcpy(ptr, str, len);

    ut_a(len <= str_len);

    str_len -= len;
    str += len;
    lsn += lsn_diff;
    ptr += lsn_diff;

    ut_a(log_lsn_validate(lsn));

    if (ptr >= buf_end) {
      /* Wrap - next copy operation will write at the
      beginning of the log buffer. */

      ptr -= log.buf_size;
    }

    if (lsn_diff > left) {
      /* We have crossed boundaries between consecutive log
      blocks. Either we finish in next block, in which case
      user will set the proper first_rec_group field after
      this function is finished, or we finish even further,
      in which case next block should have 0. In both cases,
      we reset next block's value to 0 now, and in the first
      case, user will simply overwrite it afterwards. */

      ut_a((uintptr_t(ptr) % OS_FILE_LOG_BLOCK_SIZE) == LOG_BLOCK_HDR_SIZE);

      ut_a((uintptr_t(ptr) & ~uintptr_t(LOG_BLOCK_HDR_SIZE)) %
               OS_FILE_LOG_BLOCK_SIZE ==
           0);

      log_block_set_first_rec_group(
          reinterpret_cast<byte *>(uintptr_t(ptr) &
                                   ~uintptr_t(LOG_BLOCK_HDR_SIZE)),
          0);

      if (str_len == 0) {
        /* We have finished at the boundary. */
        break;
      }

    } else {
      /* Nothing more to copy - we have finished! */
      break;
    }
  }

  ut_a(ptr >= log.buf);
  ut_a(ptr <= buf_end);
  ut_a(buf_end == log.buf + log.buf_size);
  ut_a(log_translate_lsn_to_sn(lsn) == end_sn);

  return (lsn);
}

