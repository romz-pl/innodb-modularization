#pragma once

#include <innodb/univ/univ.h>

/** status bit used for trx_undo_prev_version_build() */

/** TRX_UNDO_PREV_IN_PURGE tells trx_undo_prev_version_build() that it
is being called purge view and we would like to get the purge record
even it is in the purge view (in normal case, it will return without
fetching the purge record */
#define TRX_UNDO_PREV_IN_PURGE 0x1

/** This tells trx_undo_prev_version_build() to fetch the old value in
the undo log (which is the after image for an update) */
#define TRX_UNDO_GET_OLD_V_VALUE 0x2



/* Types of an undo log record: these have to be smaller than 16, as the
compilation info multiplied by 16 is ORed to this value in an undo log
record */

#define TRX_UNDO_INSERT_REC 11 /* fresh insert into clustered index */
#define TRX_UNDO_UPD_EXIST_REC        \
  12 /* update of a non-delete-marked \
     record */
#define TRX_UNDO_UPD_DEL_REC                \
  13 /* update of a delete marked record to \
     a not delete marked record; also the   \
     fields of the record can change */
#define TRX_UNDO_DEL_MARK_REC              \
  14 /* delete marking of a record; fields \
     do not change */
#define TRX_UNDO_CMPL_INFO_MULT           \
  16 /* compilation info is multiplied by \
     this and ORed to the type above */

#define TRX_UNDO_MODIFY_BLOB              \
  64 /* If this bit is set in type_cmpl,  \
     then the undo log record has support \
     for partial update of BLOBs. Also to \
     make the undo log format extensible, \
     introducing a new flag next to the   \
     type_cmpl flag. */

#define TRX_UNDO_UPD_EXTERN                \
  128 /* This bit can be ORed to type_cmpl \
      to denote that we updated external   \
      storage fields: used by purge to     \
      free the external storage */

/* Operation type flags used in trx_undo_report_row_operation */
#define TRX_UNDO_INSERT_OP 1
#define TRX_UNDO_MODIFY_OP 2
