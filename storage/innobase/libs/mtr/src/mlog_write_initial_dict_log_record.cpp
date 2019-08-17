#include <innodb/mtr/mlog_write_initial_dict_log_record.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/machine/data.h>


/** Writes a log record about a dictionary operation.
@param[in]	type		redo log record type
@param[in]	id		table id
@param[in]	version		table dynamic metadata version
@param[in,out]	log_ptr		current end of mini-transaction log
@param[in,out]	mtr		mini-transaction
@return end of mini-transaction log */
byte *mlog_write_initial_dict_log_record(mlog_id_t type, table_id_t id,
                                         uint64_t version, byte *log_ptr,
                                         mtr_t *mtr) {
  ut_ad(type <= MLOG_BIGGEST_TYPE);
  ut_ad(type == MLOG_TABLE_DYNAMIC_META);

  mach_write_to_1(log_ptr, type);
  log_ptr++;

  log_ptr += mach_u64_write_much_compressed(log_ptr, id);
  log_ptr += mach_u64_write_much_compressed(log_ptr, version);

  mtr->added_rec();
  return (log_ptr);
}
