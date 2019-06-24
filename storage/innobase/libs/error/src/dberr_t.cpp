#include <innodb/error/dberr_t.h>
#include <innodb/error/ut_error.h>

/** Convert an error number to a human readable text message.
The returned string is static and should not be freed or modified.
@param[in]	num	InnoDB internal error number
@return string, describing the error */
const char *ut_strerr(dberr_t num) {
  switch (num) {
    case DB_SUCCESS:
      return ("Success");
    case DB_SUCCESS_LOCKED_REC:
      return ("Success, record lock created");
    case DB_SKIP_LOCKED:
      return ("Skip locked records");
    case DB_LOCK_NOWAIT:
      return ("Don't wait for locks");
    case DB_ERROR:
      return ("Generic error");
    case DB_READ_ONLY:
      return ("Read only transaction");
    case DB_INTERRUPTED:
      return ("Operation interrupted");
    case DB_OUT_OF_MEMORY:
      return ("Cannot allocate memory");
    case DB_OUT_OF_FILE_SPACE:
    case DB_OUT_OF_DISK_SPACE:
      return ("Out of disk space");
    case DB_LOCK_WAIT:
      return ("Lock wait");
    case DB_DEADLOCK:
      return ("Deadlock");
    case DB_ROLLBACK:
      return ("Rollback");
    case DB_DUPLICATE_KEY:
      return ("Duplicate key");
    case DB_MISSING_HISTORY:
      return ("Required history data has been deleted");
    case DB_CLUSTER_NOT_FOUND:
      return ("Cluster not found");
    case DB_TABLE_NOT_FOUND:
      return ("Table not found");
    case DB_MUST_GET_MORE_FILE_SPACE:
      return ("More file space needed");
    case DB_TABLE_IS_BEING_USED:
      return ("Table is being used");
    case DB_TOO_BIG_RECORD:
      return ("Record too big");
    case DB_TOO_BIG_INDEX_COL:
      return ("Index columns size too big");
    case DB_LOCK_WAIT_TIMEOUT:
      return ("Lock wait timeout");
    case DB_NO_REFERENCED_ROW:
      return ("Referenced key value not found");
    case DB_ROW_IS_REFERENCED:
      return ("Row is referenced");
    case DB_CANNOT_ADD_CONSTRAINT:
      return ("Cannot add constraint");
    case DB_CORRUPTION:
      return ("Data structure corruption");
    case DB_CANNOT_DROP_CONSTRAINT:
      return ("Cannot drop constraint");
    case DB_NO_SAVEPOINT:
      return ("No such savepoint");
    case DB_TABLESPACE_EXISTS:
      return ("Tablespace already exists");
    case DB_TABLESPACE_DELETED:
      return ("Tablespace deleted or being deleted");
    case DB_TABLESPACE_NOT_FOUND:
      return ("Tablespace not found");
    case DB_LOCK_TABLE_FULL:
      return ("Lock structs have exhausted the buffer pool");
    case DB_FOREIGN_DUPLICATE_KEY:
      return ("Foreign key activated with duplicate keys");
    case DB_FOREIGN_EXCEED_MAX_CASCADE:
      return ("Foreign key cascade delete/update exceeds max depth");
    case DB_TOO_MANY_CONCURRENT_TRXS:
      return ("Too many concurrent transactions");
    case DB_UNSUPPORTED:
      return ("Unsupported");
    case DB_INVALID_NULL:
      return ("NULL value encountered in NOT NULL column");
    case DB_STATS_DO_NOT_EXIST:
      return ("Persistent statistics do not exist");
    case DB_FAIL:
      return ("Failed, retry may succeed");
    case DB_OVERFLOW:
      return ("Overflow");
    case DB_UNDERFLOW:
      return ("Underflow");
    case DB_STRONG_FAIL:
      return ("Failed, retry will not succeed");
    case DB_ZIP_OVERFLOW:
      return ("Zip overflow");
    case DB_RECORD_NOT_FOUND:
      return ("Record not found");
    case DB_CHILD_NO_INDEX:
      return ("No index on referencing keys in referencing table");
    case DB_PARENT_NO_INDEX:
      return ("No index on referenced keys in referenced table");
    case DB_FTS_INVALID_DOCID:
      return ("FTS Doc ID cannot be zero");
    case DB_INDEX_CORRUPT:
      return ("Index corrupted");
    case DB_UNDO_RECORD_TOO_BIG:
      return ("Undo record too big");
    case DB_END_OF_INDEX:
      return ("End of index");
    case DB_END_OF_BLOCK:
      return ("End of block");
    case DB_IO_ERROR:
      return ("I/O error");
    case DB_TABLE_IN_FK_CHECK:
      return ("Table is being used in foreign key check");
    case DB_DATA_MISMATCH:
      return ("data mismatch");
    case DB_NOT_FOUND:
      return ("not found");
    case DB_ONLINE_LOG_TOO_BIG:
      return ("Log size exceeded during online index creation");
    case DB_IDENTIFIER_TOO_LONG:
      return ("Identifier name is too long");
    case DB_FTS_EXCEED_RESULT_CACHE_LIMIT:
      return ("FTS query exceeds result cache limit");
    case DB_TEMP_FILE_WRITE_FAIL:
      return ("Temp file write failure");
    case DB_CANT_CREATE_GEOMETRY_OBJECT:
      return ("Can't create specificed geometry data object");
    case DB_CANNOT_OPEN_FILE:
      return ("Cannot open a file");
    case DB_TABLE_CORRUPT:
      return ("Table is corrupted");
    case DB_FTS_TOO_MANY_WORDS_IN_PHRASE:
      return ("Too many words in a FTS phrase or proximity search");
    case DB_IO_DECOMPRESS_FAIL:
      return ("Page decompress failed after reading from disk");
    case DB_IO_NO_PUNCH_HOLE:
      return ("No punch hole support");
    case DB_IO_NO_PUNCH_HOLE_FS:
      return ("Punch hole not supported by the file system");
    case DB_IO_NO_PUNCH_HOLE_TABLESPACE:
      return ("Punch hole not supported by the tablespace");
    case DB_IO_NO_ENCRYPT_TABLESPACE:
      return ("Page encryption not supported by the tablespace");
    case DB_IO_DECRYPT_FAIL:
      return ("Page decryption failed after reading from disk");
    case DB_IO_PARTIAL_FAILED:
      return ("Partial IO failed");
    case DB_FORCED_ABORT:
      return (
          "Transaction aborted by another higher priority "
          "transaction");
    case DB_WRONG_FILE_NAME:
      return ("Invalid Filename");
    case DB_NO_FK_ON_S_BASE_COL:
      return (
          "Cannot add foreign key on the base column "
          "of stored column");
    case DB_COMPUTE_VALUE_FAILED:
      return ("Compute generated column failed");

    case DB_INVALID_ENCRYPTION_META:
      return ("Invalid encryption meta-data information");

    case DB_ABORT_INCOMPLETE_CLONE:
      return ("Incomplete cloned data directory");

    case DB_SERVER_VERSION_LOW:
      return (
          "Cannot boot server with lower version than that built the "
          "tablespace");

    case DB_NO_SESSION_TEMP:
      return ("No session temporary tablespace allocated");

    case DB_ERROR_UNSET:;
      /* Fall through. */

      /* do not add default: in order to produce a warning if new code
      is added to the enum but not added here */
  }

  /* we abort here because if unknown error code is given, this could
  mean that memory corruption has happened and someone's error-code
  variable has been overwritten with bogus data */
  ut_error;
}
