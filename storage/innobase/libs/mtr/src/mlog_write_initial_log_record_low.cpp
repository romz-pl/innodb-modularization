#include <innodb/mtr/mlog_write_initial_log_record_low.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/machine/data.h>

/** Writes a log record about an operation.
@param[in]	type		redo log record type
@param[in]	space_id	tablespace identifier
@param[in]	page_no		page number
@param[in,out]	log_ptr		current end of mini-transaction log
@param[in,out]	mtr		mini-transaction
@return	end of mini-transaction log */
byte *mlog_write_initial_log_record_low(mlog_id_t type, space_id_t space_id,
                                        page_no_t page_no, byte *log_ptr,
                                        mtr_t *mtr) {
  ut_ad(type <= MLOG_BIGGEST_TYPE);

  mach_write_to_1(log_ptr, type);
  log_ptr++;

  log_ptr += mach_write_compressed(log_ptr, space_id);
  log_ptr += mach_write_compressed(log_ptr, page_no);

  mtr->added_rec();
  return (log_ptr);
}
