#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>

struct mtr_t;

/** Writes a log record about a dictionary operation.
@param[in]	type		redo log record type
@param[in]	id		table id
@param[in]	version		table dynamic metadata version
@param[in,out]	log_ptr		current end of mini-transaction log
@param[in,out]	mtr		mini-transaction
@return end of mini-transaction log */
byte *mlog_write_initial_dict_log_record(mlog_id_t type, table_id_t id,
                                         uint64_t version, byte *log_ptr,
                                         mtr_t *mtr);
