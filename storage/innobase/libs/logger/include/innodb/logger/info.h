#pragma once

#include <innodb/univ/univ.h>
#include <innodb/logger/logger.h>

namespace ib {

/** The class info is used to emit informational log messages.  It is to be
used similar to std::cout.  But the log messages will be emitted only when
the dtor is called.  The preferred usage of this class is to make use of
unnamed temporaries as follows:

info() << "The server started successfully.";

In the above usage, the temporary object will be destroyed at the end of the
statement and hence the log message will be emitted at the end of the
statement.  If a named object is created, then the log message will be emitted
only when it goes out of scope or destroyed. */
class info : public logger {
 public:
#ifndef UNIV_NO_ERR_MSGS

  /** Default constructor uses ER_IB_MSG_0 */
  info() : logger(INFORMATION_LEVEL) {}

  /** Constructor.
  @param[in]	err		Error code from errmsg-*.txt.
  @param[in]	args		Variable length argument list */
  template <class... Args>
  explicit info(int err, Args &&... args)
      : logger(INFORMATION_LEVEL, err, std::forward<Args>(args)...) {}
#else
  /** Destructor */
  ~info();
#endif /* !UNIV_NO_ERR_MSGS */
};

}
