#include <innodb/page/page_create_write_log.h>

#include <innodb/log_types/mlog_id_t.h>
#include <innodb/mtr/mlog_write_initial_log_record.h>
#include <innodb/error/ut_error.h>

#ifndef UNIV_HOTBACKUP

/** Writes a log record of page creation
@param[in]	frame		a buffer frame where the page is created
@param[in]	mtr		mini-transaction handle
@param[in]	comp		TRUE=compact page format
@param[in]	page_type	page type */
void page_create_write_log(buf_frame_t *frame, mtr_t *mtr, ibool comp,
                           page_type_t page_type) {
  mlog_id_t type;

  switch (page_type) {
    case FIL_PAGE_INDEX:
      type = comp ? MLOG_COMP_PAGE_CREATE : MLOG_PAGE_CREATE;
      break;
    case FIL_PAGE_RTREE:
      type = comp ? MLOG_COMP_PAGE_CREATE_RTREE : MLOG_PAGE_CREATE_RTREE;
      break;
    case FIL_PAGE_SDI:
      type = comp ? MLOG_COMP_PAGE_CREATE_SDI : MLOG_PAGE_CREATE_SDI;
      break;
    default:
      ut_error;
  }

  mlog_write_initial_log_record(frame, type, mtr);
}

#endif /* !UNIV_HOTBACKUP */
