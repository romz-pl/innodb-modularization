#include <innodb/log_recv/log_block_checksum_is_ok.h>

#include <innodb/log_block/log_block_get_checksum.h>
#include <innodb/log_block/log_block_calc_checksum.h>

/** Whether to generate and require checksums on the redo log pages. */
extern bool srv_log_checksums;

/** Check the 4-byte checksum to the trailer checksum field of a log
block.
@param[in]	block	pointer to a log block
@return whether the checksum matches */
bool log_block_checksum_is_ok(const byte *block) {
  return (!srv_log_checksums ||
          log_block_get_checksum(block) == log_block_calc_checksum(block));
}
