#pragma once

#include <innodb/univ/univ.h>

/** File types */
enum fil_type_t : uint8_t {
  /** temporary tablespace (temporary undo log or tables) */
  FIL_TYPE_TEMPORARY = 1,
  /** a tablespace that is being imported (no logging until finished) */
  FIL_TYPE_IMPORT = 2,
  /** persistent tablespace (for system, undo log or tables) */
  FIL_TYPE_TABLESPACE = 4,
  /** redo log covering changes to files of FIL_TYPE_TABLESPACE */
  FIL_TYPE_LOG = 8
};


/** Check if fil_type is any of FIL_TYPE_TEMPORARY, FIL_TYPE_IMPORT
or FIL_TYPE_TABLESPACE.
@param[in]	type	variable of type fil_type_t
@return true if any of FIL_TYPE_TEMPORARY, FIL_TYPE_IMPORT
or FIL_TYPE_TABLESPACE */
inline bool fil_type_is_data(fil_type_t type) {
  return (type == FIL_TYPE_TEMPORARY || type == FIL_TYPE_IMPORT ||
          type == FIL_TYPE_TABLESPACE);
}
