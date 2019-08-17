#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>

struct mtr_t;

/** Parses an initial log record written by mlog_write_initial_dict_log_record.
@param[in]	ptr		buffer
@param[in]	end_ptr		buffer end
@param[out]	type		log record type, should be
                                MLOG_TABLE_DYNAMIC_META
@param[out]	id		table id
@param[out]	version		table dynamic metadata version
@return parsed record end, NULL if not a complete record */
byte *mlog_parse_initial_dict_log_record(const byte *ptr, const byte *end_ptr,
                                         mlog_id_t *type, table_id_t *id,
                                         uint64 *version);
