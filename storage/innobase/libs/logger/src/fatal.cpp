#include <innodb/logger/fatal.h>
#include <innodb/error/ut_error.h>
#include "mysql/components/services/log_builtins.h"

//
// This is defined in file ut0ut.cc
// Moving the code here causes linker error, cince the circular deendency!
//
#if 0

#if !defined(UNIV_HOTBACKUP) && !defined(UNIV_NO_ERR_MSGS)

namespace ib {

fatal::~fatal() {
  auto s = m_oss.str();

  LogEvent()
      .type(LOG_TYPE_ERROR)
      .prio(m_level)
      .errcode(m_err)
      .subsys("InnoDB")
      .verbatim(s.c_str());

  ut_error;
}

}

#endif

#endif
