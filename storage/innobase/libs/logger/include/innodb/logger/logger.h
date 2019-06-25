#pragma once

#include <sstream>

#include <innodb/univ/univ.h>
#include <innodb/assert/assert.h>
#include <innodb/logger/srv_get_server_errmsgs.h>

#include "mysqld_error.h"
#include <mysql/components/services/log_shared.h>

namespace ib {

/** The class logger is the base class of all the error log related classes.
It contains a std::ostringstream object.  The main purpose of this class is
to forward operator<< to the underlying std::ostringstream object.  Do not
use this class directly, instead use one of the derived classes. */
class logger {
 public:
  /** Destructor */
  virtual ~logger();

#ifndef UNIV_NO_ERR_MSGS

  /** Format an error message.
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  logger &log(int err, Args &&... args) {
    ut_a(m_err == ER_IB_MSG_0);

    m_err = err;

    m_oss << msg(err, std::forward<Args>(args)...);

    return (*this);
  }

#endif /* !UNIV_NO_ERR_MSGS */

  template <typename T>
  logger &operator<<(const T &rhs) {
    m_oss << rhs;
    return (*this);
  }

  /** Write the given buffer to the internal string stream object.
  @param[in]	buf		the buffer contents to log.
  @param[in]	count		the length of the buffer buf.
  @return the output stream into which buffer was written. */
  std::ostream &write(const char *buf, std::streamsize count) {
    return (m_oss.write(buf, count));
  }

  /** Write the given buffer to the internal string stream object.
  @param[in]	buf		the buffer contents to log
  @param[in]	count		the length of the buffer buf.
  @return the output stream into which buffer was written. */
  std::ostream &write(const byte *buf, std::streamsize count) {
    return (m_oss.write(reinterpret_cast<const char *>(buf), count));
  }

 public:
  /** For converting the message into a string. */
  std::ostringstream m_oss;

#ifndef UNIV_NO_ERR_MSGS
  /** Error code in errmsg-*.txt */
  int m_err{};

  /** Error logging level. */
  loglevel m_level{INFORMATION_LEVEL};
#endif /* !UNIV_NO_ERR_MSGS */

#ifdef UNIV_HOTBACKUP
  /** For MEB trace infrastructure. */
  int m_trace_level{};
#endif /* UNIV_HOTBACKUP */

 protected:
#ifndef UNIV_NO_ERR_MSGS
  /** Format an error message.
  @param[in]	err	Error code from errmsg-*.txt.
  @param[in]	args	Variable length argument list */
  template <class... Args>
  static std::string msg(int err, Args &&... args) {
    const char *fmt = srv_get_server_errmsgs(err);

    int ret;
    char buf[LOG_BUFF_MAX];
#ifdef UNIV_DEBUG
    if (get_first_format(fmt) != nullptr) {
      if (!verify_fmt_match(fmt, std::forward<Args>(args)...)) {
        fprintf(stderr, "The format '%s' does not match arguments\n", fmt);
        ut_error;
      }
    }
#endif
    ret = snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);

    std::string str;

    if (ret > 0 && (size_t)ret < sizeof(buf)) {
      str.append(buf);
    }

    return (str);
  }

 protected:
  /** Constructor.
  @param[in]	level		Logging level
  @param[in]	err		Error message code. */
  logger(loglevel level, int err) : m_err(err), m_level(level) {
    /* Note: Dummy argument to avoid the warning:

    "format not a string literal and no format arguments"
    "[-Wformat-security]"

    The warning only kicks in if the call is of the form:

       snprintf(buf, sizeof(buf), str);
    */

    m_oss << msg(err, "");
  }

  /** Constructor.
  @param[in]	level		Logging level
  @param[in]	err		Error message code.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit logger(loglevel level, int err, Args &&... args)
      : m_err(err), m_level(level) {
    m_oss << msg(err, std::forward<Args>(args)...);
  }

  /** Constructor
  @param[in]	level		Log error level */
  explicit logger(loglevel level) : m_err(ER_IB_MSG_0), m_level(level) {}

#endif /* !UNIV_NO_ERR_MSGS */
};

}
