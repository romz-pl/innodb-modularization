#include <innodb/tablespace/fil_write_flushed_lsn.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/tablespace/fil_system.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/align/ut_align.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/enum/to_int.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/tablespace/fil_system.h>
#include <innodb/tablespace/fil_read.h>
#include <innodb/tablespace/fil_write.h>
#include <innodb/machine/data.h>
#include <innodb/trx_sys/flags.h>

/** Write the flushed LSN to the page header of the first page in the
system tablespace.
@param[in]	lsn	flushed LSN
@return DB_SUCCESS or error number */
dberr_t fil_write_flushed_lsn(lsn_t lsn) {
  byte *buf1;
  byte *buf;
  dberr_t err;

  buf1 = static_cast<byte *>(ut_malloc_nokey(2 * UNIV_PAGE_SIZE));
  buf = static_cast<byte *>(ut_align(buf1, UNIV_PAGE_SIZE));

  const page_id_t page_id(TRX_SYS_SPACE, 0);

  err = fil_read(page_id, univ_page_size, 0, univ_page_size.physical(), buf);

  if (err == DB_SUCCESS) {
    mach_write_to_8(buf + FIL_PAGE_FILE_FLUSH_LSN, lsn);

    err = fil_write(page_id, univ_page_size, 0, univ_page_size.physical(), buf);

    fil_system->flush_file_spaces(to_int(FIL_TYPE_TABLESPACE));
  }

  ut_free(buf1);

  return (err);
}


#endif
