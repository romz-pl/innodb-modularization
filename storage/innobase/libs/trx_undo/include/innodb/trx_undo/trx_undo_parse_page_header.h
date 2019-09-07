#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>
#include <innodb/disk/page_t.h>

struct mtr_t;

/** Parse the redo log entry of an undo log page header create or reuse.
@param[in]	type	MLOG_UNDO_HDR_CREATE or MLOG_UNDO_HDR_REUSE
@param[in]	ptr	redo log record
@param[in]	end_ptr	end of log buffer
@param[in,out]	page	page frame or NULL
@param[in,out]	mtr	mini-transaction or NULL
@return end of log record or NULL */
byte *trx_undo_parse_page_header(mlog_id_t type, const byte *ptr,
                                 const byte *end_ptr, page_t *page, mtr_t *mtr);
