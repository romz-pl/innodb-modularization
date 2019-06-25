#pragma once

#ifdef UNIV_HOTBACKUP

#include <innodb/univ/univ.h>
#include <innodb/logger/logger.h>

namespace ib {
/**  The class trace is used to emit informational log messages. only when
trace level is set in the MEB code */
class trace_1 : public logger {
 public:
#ifndef UNIV_NO_ERR_MSGS
  /** Default constructor uses ER_IB_MSG_0 */
  trace_1() : logger(INFORMATION_LEVEL) { m_trace_level = 1; }

  /** Constructor.
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit trace_1(int err, Args &&... args)
      : logger(INFORMATION_LEVEL, err, std::forward<Args>(args)...) {
    m_trace_level = 1;
  }

#else
  /** Constructor */
  trace_1();
#endif /* !UNIV_NO_ERR_MSGS */
};
}

#endif

