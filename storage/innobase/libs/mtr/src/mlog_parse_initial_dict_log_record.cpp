#include <innodb/mtr/mlog_parse_initial_dict_log_record.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/machine/data.h>

/** Parse an initial log record written by mlog_write_initial_dict_log_record.
@param[in]	ptr		buffer
@param[in]	end_ptr		buffer end
@param[out]	type		log record type, should be
                                MLOG_TABLE_DYNAMIC_META
@param[out]	id		table id
@param[out]	version		table dynamic metadata version
@return parsed record end, NULL if not a complete record */
byte *mlog_parse_initial_dict_log_record(const byte *ptr, const byte *end_ptr,
                                         mlog_id_t *type, table_id_t *id,
                                         uint64 *version) {
  if (end_ptr < ptr + 1) {
    return (nullptr);
  }

  *type = (mlog_id_t)((ulint)*ptr & ~MLOG_SINGLE_REC_FLAG);
  ut_ad(*type == MLOG_TABLE_DYNAMIC_META);

  ptr++;

  if (end_ptr < ptr + 1) {
    return (nullptr);
  }

  *id = mach_parse_u64_much_compressed(&ptr, end_ptr);

  if (ptr == nullptr || end_ptr < ptr + 1) {
    return (nullptr);
  }

  *version = mach_parse_u64_much_compressed(&ptr, end_ptr);

  return (const_cast<byte *>(ptr));
}
