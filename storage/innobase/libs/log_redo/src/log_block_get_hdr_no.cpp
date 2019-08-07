#include <innodb/log_redo/log_block_get_hdr_no.h>

#include <innodb/log_redo/flags.h>
#include <innodb/machine/data.h>

uint32_t log_block_get_hdr_no(const byte *log_block) {
  return (~LOG_BLOCK_FLUSH_BIT_MASK &
          mach_read_from_4(log_block + LOG_BLOCK_HDR_NO));
}
