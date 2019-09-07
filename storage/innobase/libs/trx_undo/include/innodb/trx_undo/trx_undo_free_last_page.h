#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_undo/trx_undo_free_last_page_func.h>

#ifdef UNIV_DEBUG

#define trx_undo_free_last_page(trx, undo, mtr) \
  trx_undo_free_last_page_func(trx, undo, mtr)

#else /* UNIV_DEBUG */

#define trx_undo_free_last_page(trx, undo, mtr) \
  trx_undo_free_last_page_func(undo, mtr)

#endif /* UNIV_DEBUG */
