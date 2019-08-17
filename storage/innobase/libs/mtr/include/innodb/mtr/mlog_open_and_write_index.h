#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>

struct mtr_t;
struct dict_index_t;

/** Opens a buffer for mlog, writes the initial log record and,
 if needed, the field lengths of an index.  Reserves space
 for further log entries.  The log entry must be closed with
 mtr_close().
 @return buffer, NULL if log mode MTR_LOG_NONE */
byte *mlog_open_and_write_index(
    mtr_t *mtr,                /*!< in: mtr */
    const byte *rec,           /*!< in: index record or page */
    const dict_index_t *index, /*!< in: record descriptor */
    mlog_id_t type,            /*!< in: log item type */
    ulint size);               /*!< in: requested buffer size in bytes
                               (if 0, calls mlog_close() and
                               returns NULL) */
