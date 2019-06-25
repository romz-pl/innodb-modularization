#pragma once

#include <innodb/univ/univ.h>
#include <innodb/logger/logger.h>

namespace ib {

/** Emit a fatal message if the given predicate is true, otherwise emit a
error message. */
class fatal_or_error : public logger {
 public:
#ifndef UNIV_NO_ERR_MSGS
  /** Default constructor uses ER_IB_MSG_0
  @param[in]	fatal		true if it's a fatal message */
  fatal_or_error(bool fatal) : logger(ERROR_LEVEL), m_fatal(fatal) {
    if (m_fatal) {
      m_oss << "[fatal]";
    }
  }

  /** Constructor.
  @param[in]	fatal		true if it's a fatal message
  @param[in]	err		Error code from errmsg-*.txt. */
  template <class... Args>
  explicit fatal_or_error(bool fatal, int err)
      : logger(ERROR_LEVEL, err), m_fatal(fatal) {
    if (m_fatal) {
      m_oss << "[fatal]";
    }

    m_oss << msg(err, "");
  }

  /** Constructor.
  @param[in]	fatal		true if it's a fatal message
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit fatal_or_error(bool fatal, int err, Args &&... args)
      : logger(ERROR_LEVEL, err), m_fatal(fatal) {
    if (m_fatal) {
      m_oss << "[fatal]";
    }

    m_oss << msg(err, std::forward<Args>(args)...);
  }

  /** Destructor */
  virtual ~fatal_or_error();
#else
  /** Constructor */
  fatal_or_error(bool fatal) : m_fatal(fatal) {}
#endif /* !UNIV_NO_ERR_MSGS */
 private:
  /** If true then assert after printing an error message. */
  const bool m_fatal;
};

}
