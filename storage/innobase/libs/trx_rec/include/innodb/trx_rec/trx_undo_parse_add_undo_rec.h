#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Parses a redo log record of adding an undo log record.
 @return end of log record or NULL */
byte *trx_undo_parse_add_undo_rec(byte *ptr,     /*!< in: buffer */
                                  byte *end_ptr, /*!< in: buffer end */
                                  page_t *page); /*!< in: page or NULL */
