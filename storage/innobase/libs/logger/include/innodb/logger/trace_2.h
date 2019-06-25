#pragma once

#ifdef UNIV_HOTBACKUP

#include <innodb/univ/univ.h>
#include <innodb/logger/logger.h>

namespace ib {

/**  The class trace_2 is used to emit informational log messages only when
trace level 2 is set in the MEB code */
class trace_2 : public logger {
 public:
#ifndef UNIV_NO_ERR_MSGS
  /** Default constructor uses ER_IB_MSG_0 */
  trace_2() : logger(INFORMATION_LEVEL) { m_trace_level = 2; }

  /** Constructor.
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit trace_2(int err, Args &&... args)
      : logger(INFORMATION_LEVEL, err, std::forward<Args>(args)...) {
    m_trace_level = 2;
  }
#else
  /** Destructor. */
  trace_2();
#endif /* !UNIV_NO_ERR_MSGS */
};
}

#endif
