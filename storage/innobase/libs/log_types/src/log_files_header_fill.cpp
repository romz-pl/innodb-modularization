#include <innodb/log_types/log_files_header_fill.h>

#include <innodb/log_types/flags.h>
#include <innodb/log_types/log_block_set_checksum.h>
#include <innodb/log_types/log_block_calc_checksum_crc32.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/log_types/log_header_format_t.h>

void log_files_header_fill(byte *buf, lsn_t start_lsn, const char *creator) {
  memset(buf, 0, OS_FILE_LOG_BLOCK_SIZE);

  mach_write_to_4(buf + LOG_HEADER_FORMAT, LOG_HEADER_FORMAT_CURRENT);
  mach_write_to_8(buf + LOG_HEADER_START_LSN, start_lsn);

  strncpy(reinterpret_cast<char *>(buf) + LOG_HEADER_CREATOR, creator,
          LOG_HEADER_CREATOR_END - LOG_HEADER_CREATOR);

  ut_ad(LOG_HEADER_CREATOR_END - LOG_HEADER_CREATOR >= strlen(creator));

  log_block_set_checksum(buf, log_block_calc_checksum_crc32(buf));
}
