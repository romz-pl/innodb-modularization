#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

/** Converts a lsn to a log block number. Consecutive log blocks have
consecutive numbers (unless the sequence wraps). It is guaranteed that
the calculated number is greater than zero.

@param[in]	lsn	lsn of a byte within the block
@return log block number, it is > 0 and <= 1G */
uint32_t log_block_convert_lsn_to_no(lsn_t lsn);
