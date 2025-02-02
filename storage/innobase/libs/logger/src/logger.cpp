#include <innodb/logger/logger.h>
#include "mysql/components/services/log_builtins.h"

//
// This is defined in file ut0ut.cc
// Moving the code here causes linker error: undefined reference to 'log_bi' and undefined reference to 'log_bs'
//
#if 0

#if !defined(UNIV_HOTBACKUP) && !defined(UNIV_NO_ERR_MSGS)

namespace ib {

logger::~logger() {
  auto s = m_oss.str();

  LogEvent()
      .type(LOG_TYPE_ERROR)
      .prio(m_level)
      .errcode(m_err)
      .subsys("InnoDB")
      .verbatim(s.c_str());
}

}

#endif

#endif
