#include <innodb/log_redo/log_translate_lsn_to_sn.h>

#include <innodb/log_redo/flags.h>
#include <innodb/log_types/sn_t.h>


#ifndef UNIV_HOTBACKUP

lsn_t log_translate_lsn_to_sn(lsn_t lsn) {
  /* Calculate sn of the beginning of log block, which contains
  the provided lsn value. */
  const sn_t sn = lsn / OS_FILE_LOG_BLOCK_SIZE * LOG_BLOCK_DATA_SIZE;

  /* Calculate offset for the provided lsn within the log block.
  The offset includes LOG_BLOCK_HDR_SIZE bytes of block's header. */
  const uint32_t diff = lsn % OS_FILE_LOG_BLOCK_SIZE;

  if (diff < LOG_BLOCK_HDR_SIZE) {
    /* The lsn points to some bytes inside the block's header.
    Return sn for the beginning of the block. Note, that sn
    values don't enumerate bytes of blocks' headers, so the
    value of diff does not matter at all. */
    return (sn);
  }

  if (diff > OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE) {
    /* The lsn points to some bytes inside the block's footer.
    Return sn for the beginning of the next block. Note, that
    sn values don't enumerate bytes of blocks' footer, so the
    value of diff does not matter at all. */
    return (sn + LOG_BLOCK_DATA_SIZE);
  }

  /* Add the offset but skip bytes of block's header. */
  return (sn + diff - LOG_BLOCK_HDR_SIZE);
}


#endif
