#include <innodb/log_recv/recv_check_log_header_checksum.h>

#include <innodb/log_block/log_block_get_checksum.h>
#include <innodb/log_block/log_block_calc_checksum_crc32.h>

/** Check the consistency of a log header block.
@param[in]	buf	header block
@return true if ok */
bool recv_check_log_header_checksum(const byte *buf) {
  auto c1 = log_block_get_checksum(buf);
  auto c2 = log_block_calc_checksum_crc32(buf);

  return (c1 == c2);
}
