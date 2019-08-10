#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

/** Validates a given lsn value. Checks if the lsn value points to data
bytes inside log block (not to some bytes in header/footer). It is used
by assertions.
@return true if lsn points to data bytes within log block */
bool log_lsn_validate(lsn_t lsn);
