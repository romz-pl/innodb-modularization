#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Read 1 - 4 bytes from a file page buffered in the buffer pool.
@return	value read */
#define mtr_read_ulint(p, t, m) (m)->read_ulint((p), (t))
