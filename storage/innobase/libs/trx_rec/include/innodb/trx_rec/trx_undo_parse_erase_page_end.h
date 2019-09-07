#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct mtr_t;

/** Parses a redo log record of erasing of an undo page end.
 @return end of log record or NULL */
byte *trx_undo_parse_erase_page_end(byte *ptr,     /*!< in: buffer */
                                    byte *end_ptr, /*!< in: buffer end */
                                    page_t *page,  /*!< in: page or NULL */
                                    mtr_t *mtr);   /*!< in: mtr or NULL */
