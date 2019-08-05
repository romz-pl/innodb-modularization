#include <innodb/tablespace/fil_discard_tablespace.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/tablespace/fil_delete_tablespace.h>
#include <innodb/logger/warn.h>
#include <innodb/error/ut_error.h>

void ibuf_delete_for_discarded_space(space_id_t space);

/** Discards a single-table tablespace. The tablespace must be cached in the
memory cache. Discarding is like deleting a tablespace, but

 1. We do not drop the table from the data dictionary;

 2. We remove all insert buffer entries for the tablespace immediately;
    in DROP TABLE they are only removed gradually in the background;

 3. Free all the pages in use by the tablespace.
@param[in]	space_id		Tablespace ID
@return DB_SUCCESS or error */
dberr_t fil_discard_tablespace(space_id_t space_id) {
  dberr_t err;

  err = fil_delete_tablespace(space_id, BUF_REMOVE_ALL_NO_WRITE);

  switch (err) {
    case DB_SUCCESS:
      break;

    case DB_IO_ERROR:

      ib::warn(ER_IB_MSG_291, ulong{space_id}, ut_strerr(err));
      break;

    case DB_TABLESPACE_NOT_FOUND:

      ib::warn(ER_IB_MSG_292, ulong{space_id}, ut_strerr(err));
      break;

    default:
      ut_error;
  }

  /* Remove all insert buffer entries for the tablespace */

  ibuf_delete_for_discarded_space(space_id);

  return (err);
}


#endif
