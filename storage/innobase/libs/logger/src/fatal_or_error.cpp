#include <innodb/logger/fatal_or_error.h>

//
// This is defined in file ut0ut.cc
// Moving the code here causes linker error, cince the circular deendency!
//
//#if 0

#if !defined(UNIV_HOTBACKUP) && !defined(UNIV_NO_ERR_MSGS)

namespace ib {

fatal_or_error::~fatal_or_error() {
    ut_a(!m_fatal);
}

}  // namespace ib

#endif

//#endif
