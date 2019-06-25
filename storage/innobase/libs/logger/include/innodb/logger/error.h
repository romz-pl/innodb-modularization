#pragma once

#include <innodb/univ/univ.h>
#include <innodb/logger/logger.h>

namespace ib {

/** The class error is used to emit error messages.  Refer to the
documentation of class info for further details. */
class error : public logger {
 public:
#ifndef UNIV_NO_ERR_MSGS
  /** Default constructor uses ER_IB_MSG_0 */
  error() : logger(ERROR_LEVEL) {}

  /** Constructor.
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit error(int err, Args &&... args)
      : logger(ERROR_LEVEL, err, std::forward<Args>(args)...) {}

#else
  /** Destructor */
  ~error();
#endif /* !UNIV_NO_ERR_MSGS */
};

}
