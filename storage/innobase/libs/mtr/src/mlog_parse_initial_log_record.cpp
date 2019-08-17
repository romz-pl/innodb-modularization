#include <innodb/mtr/mlog_parse_initial_log_record.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

/** Parses an initial log record written by mlog_write_initial_log_record.
 @return parsed record end, NULL if not a complete record */
byte *mlog_parse_initial_log_record(
    const byte *ptr,     /*!< in: buffer */
    const byte *end_ptr, /*!< in: buffer end */
    mlog_id_t *type,     /*!< out: log record type: MLOG_1BYTE, ... */
    space_id_t *space,   /*!< out: space id */
    page_no_t *page_no)  /*!< out: page number */
{
  if (end_ptr < ptr + 1) {
    return (NULL);
  }

  *type = (mlog_id_t)((ulint)*ptr & ~MLOG_SINGLE_REC_FLAG);
  ut_ad(*type <= MLOG_BIGGEST_TYPE);

  ptr++;

  if (end_ptr < ptr + 2) {
    return (NULL);
  }

  *space = mach_parse_compressed(&ptr, end_ptr);

  if (ptr != NULL) {
    *page_no = mach_parse_compressed(&ptr, end_ptr);
  }

  return (const_cast<byte *>(ptr));
}
