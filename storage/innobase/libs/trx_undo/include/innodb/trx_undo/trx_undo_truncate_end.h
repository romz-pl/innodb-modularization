#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_undo/trx_undo_truncate_end_func.h>

#ifdef UNIV_DEBUG

#define trx_undo_truncate_end(trx, undo, limit) \
  trx_undo_truncate_end_func(trx, undo, limit)

#else /* UNIV_DEBUG */

#define trx_undo_truncate_end(trx, undo, limit) \
  trx_undo_truncate_end_func(undo, limit)

#endif /* UNIV_DEBUG */
