#pragma once

#include <innodb/univ/univ.h>

/** Gets a log block number stored in the header. The number corresponds
to lsn range for data stored in the block.

During recovery, when a next block is being parsed, a next range of lsn
values is expected to be read. This corresponds to a log block number
increased by one. However, if a smaller number is read from the header,
it is then considered the end of the redo log and recovery is finished.
In such case, the next block is most likely an empty block or a block
from the past, because the redo log is written in circular manner.

@param[in]	log_block	log block (may be invalid or empty block)
@return log block number stored in the block header */
uint32_t log_block_get_hdr_no(const byte *log_block);
