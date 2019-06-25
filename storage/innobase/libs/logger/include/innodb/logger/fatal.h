#pragma once

#include <innodb/univ/univ.h>
#include <innodb/logger/logger.h>

namespace ib {

/** The class fatal is used to emit an error message and stop the server
by crashing it.  Use this class when MySQL server needs to be stopped
immediately.  Refer to the documentation of class info for usage details. */
class fatal : public logger {
 public:
#ifndef UNIV_NO_ERR_MSGS
  /** Default constructor uses ER_IB_MSG_0 */
  fatal() : logger(ERROR_LEVEL) { m_oss << "[FATAL] "; }

  /** Default constructor uses ER_IB_MSG_0 */
  explicit fatal(int err) : logger(ERROR_LEVEL) {
    m_oss << "[FATAL] ";

    m_oss << msg(err, "");
  }

  /** Constructor.
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit fatal(int err, Args &&... args) : logger(ERROR_LEVEL, err) {
    m_oss << "[FATAL] ";

    m_oss << msg(err, std::forward<Args>(args)...);
  }

  /** Destructor. */
  virtual ~fatal();
#else
  /** Destructor. */
  ~fatal();
#endif /* !UNIV_NO_ERR_MSGS */
};

}
