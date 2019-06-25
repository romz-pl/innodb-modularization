#pragma once

#include <innodb/univ/univ.h>
#include <innodb/logger/logger.h>

namespace ib {

/** Emit an error message if the given predicate is true, otherwise emit a
warning message */
class error_or_warn : public logger {
 public:
#ifndef UNIV_NO_ERR_MSGS

  /** Default constructor uses ER_IB_MSG_0
  @param[in]	pred		True if it's a warning. */
  error_or_warn(bool pred) : logger(pred ? ERROR_LEVEL : WARNING_LEVEL) {}

  /** Constructor.
  @param[in]	pred		True if it's a warning.
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit error_or_warn(bool pred, int err, Args &&... args)
      : logger(pred ? ERROR_LEVEL : WARNING_LEVEL, err,
               std::forward<Args>(args)...) {}

#endif /* !UNIV_NO_ERR_MSGS */
};

}
