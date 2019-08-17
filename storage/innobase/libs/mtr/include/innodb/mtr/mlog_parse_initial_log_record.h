#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>
#include <innodb/univ/page_no_t.h>

struct mtr_t;

/** Parses an initial log record written by mlog_write_initial_log_record.
 @return parsed record end, NULL if not a complete record */
byte *mlog_parse_initial_log_record(
    const byte *ptr,     /*!< in: buffer */
    const byte *end_ptr, /*!< in: buffer end */
    mlog_id_t *type,     /*!< out: log record type: MLOG_1BYTE, ... */
    space_id_t *space,   /*!< out: space id */
    page_no_t *page_no); /*!< out: page number */
