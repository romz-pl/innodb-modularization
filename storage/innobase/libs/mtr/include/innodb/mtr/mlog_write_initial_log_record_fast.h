#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/mlog_id_t.h>

/** Writes the initial part of a log record (3..11 bytes).
If the implementation of this function is changed, all size parameters to
mlog_open() should be adjusted accordingly!
@param[in]	ptr	pointer to (inside) a buffer frame holding the file
                        page where modification is made
@param[in]	type	log item type: MLOG_1BYTE, ...
@param[in]	log_ptr	pointer to mtr log which has been opened
@param[in]	mtr	mtr
@return new value of log_ptr */
byte *mlog_write_initial_log_record_fast(const byte *ptr, mlog_id_t type,
                                         byte *log_ptr, mtr_t *mtr);
#else /* !UNIV_HOTBACKUP */

#define mlog_write_initial_log_record_fast(ptr, type, log_ptr, mtr) ((byte *)0)

#endif /* !UNIV_HOTBACKUP */
