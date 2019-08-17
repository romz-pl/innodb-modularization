#include <innodb/mtr/mlog_open_metadata.h>

#include <innodb/mtr/mtr_t.h>

/** Opens a buffer to mlog. It must be closed with mlog_close.
This is used for writing log for metadata changes
@param[in,out]	mtr	mtr
@param[in]	size	buffer size in bytes; MUST be smaller than
                        DYN_ARRAY_DATA_SIZE!
@return buffer */
byte *mlog_open_metadata(mtr_t *mtr, ulint size) {
  return (mtr->get_log()->open(size));
}
