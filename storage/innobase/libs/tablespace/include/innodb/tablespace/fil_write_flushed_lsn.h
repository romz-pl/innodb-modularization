#pragma once

#include <innodb/univ/univ.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/log_sn/lsn_t.h>
#include <innodb/error/dberr_t.h>

/** Write the flushed LSN to the page header of the first page in the
system tablespace.
@param[in]	lsn		Flushed LSN
@return DB_SUCCESS or error number */
dberr_t fil_write_flushed_lsn(lsn_t lsn) MY_ATTRIBUTE((warn_unused_result));

#endif
