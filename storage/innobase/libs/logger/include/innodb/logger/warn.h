#pragma once

#include <innodb/univ/univ.h>
#include <innodb/logger/logger.h>

namespace ib {

/** The class warn is used to emit warnings.  Refer to the documentation of
class info for further details. */
class warn : public logger {
 public:
#ifndef UNIV_NO_ERR_MSGS
  /** Default constructor uses ER_IB_MSG_0 */
  warn() : logger(WARNING_LEVEL) {}

  /** Constructor.
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit warn(int err, Args &&... args)
      : logger(WARNING_LEVEL, err, std::forward<Args>(args)...) {}

#else
  /** Destructor */
  ~warn();
#endif /* !UNIV_NO_ERR_MSGS */
};

}
