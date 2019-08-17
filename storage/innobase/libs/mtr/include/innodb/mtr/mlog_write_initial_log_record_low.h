#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>
#include <innodb/univ/page_no_t.h>

struct mtr_t;

/** Writes a log record about an operation.
@param[in]	type		redo log record type
@param[in]	space_id	tablespace identifier
@param[in]	page_no		page number
@param[in,out]	log_ptr		current end of mini-transaction log
@param[in,out]	mtr		mini-transaction
@return	end of mini-transaction log */
byte *mlog_write_initial_log_record_low(mlog_id_t type, space_id_t space_id,
                                        page_no_t page_no, byte *log_ptr,
                                        mtr_t *mtr);
