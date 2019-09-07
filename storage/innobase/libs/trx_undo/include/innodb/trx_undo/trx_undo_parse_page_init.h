#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct mtr_t;

/** Parses the redo log entry of an undo log page initialization.
 @return end of log record or NULL */
byte *trx_undo_parse_page_init(const byte *ptr,     /*!< in: buffer */
                               const byte *end_ptr, /*!< in: buffer end */
                               page_t *page,        /*!< in: page or NULL */
                               mtr_t *mtr);         /*!< in: mtr or NULL */
