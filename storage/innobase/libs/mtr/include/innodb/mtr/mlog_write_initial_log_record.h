#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>

struct mtr_t;


#ifndef UNIV_HOTBACKUP

/** Writes initial part of a log record consisting of one-byte item
 type and four-byte space and page numbers. */
void mlog_write_initial_log_record(
    const byte *ptr, /*!< in: pointer to (inside) a buffer
                     frame holding the file page where
                     modification is made */
    mlog_id_t type,  /*!< in: log item type: MLOG_1BYTE, ... */
    mtr_t *mtr);     /*!< in: mini-transaction handle */

#else /* !UNIV_HOTBACKUP */

#define mlog_write_initial_log_record(ptr, type, mtr) ((void)0)

#endif /* !UNIV_HOTBACKUP */
