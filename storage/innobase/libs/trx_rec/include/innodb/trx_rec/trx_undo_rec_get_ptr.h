#pragma once

#include <innodb/univ/univ.h>

/** Returns the start of the undo record data area. */
#define trx_undo_rec_get_ptr(undo_rec, undo_no) \
  ((undo_rec) + trx_undo_rec_get_offset(undo_no))
