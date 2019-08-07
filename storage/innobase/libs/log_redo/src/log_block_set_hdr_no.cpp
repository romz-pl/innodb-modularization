#include <innodb/log_redo/log_block_set_hdr_no.h>

#include <innodb/log_redo/flags.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

void log_block_set_hdr_no(byte *log_block, uint32_t n) {
  ut_a(n > 0);
  ut_a(n < LOG_BLOCK_FLUSH_BIT_MASK);
  ut_a(n <= LOG_BLOCK_MAX_NO);

  mach_write_to_4(log_block + LOG_BLOCK_HDR_NO, n);
}
