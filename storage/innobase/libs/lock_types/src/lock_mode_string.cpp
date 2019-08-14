#include <innodb/lock_types/lock_mode_string.h>

#include <innodb/error/ut_error.h>

/** Convert the given enum value into string.
@param[in]	mode	the lock mode
@return human readable string of the given enum value */
const char *lock_mode_string(enum lock_mode mode) {
  switch (mode) {
    case LOCK_IS:
      return ("LOCK_IS");
    case LOCK_IX:
      return ("LOCK_IX");
    case LOCK_S:
      return ("LOCK_S");
    case LOCK_X:
      return ("LOCK_X");
    case LOCK_AUTO_INC:
      return ("LOCK_AUTO_INC");
    case LOCK_NONE:
      return ("LOCK_NONE");
    case LOCK_NONE_UNSET:
      return ("LOCK_NONE_UNSET");
    default:
      ut_error;
  }
}
