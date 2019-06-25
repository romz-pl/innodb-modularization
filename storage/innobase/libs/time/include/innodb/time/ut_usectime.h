#pragma once

#include <innodb/univ/univ.h>

/** Returns system time.
 Upon successful completion, the value 0 is returned; otherwise the
 value -1 is returned and the global variable errno is set to indicate the
 error.
 @return 0 on success, -1 otherwise */
int ut_usectime(ulint *sec, /*!< out: seconds since the Epoch */
                ulint *ms); /*!< out: microseconds since the Epoch+*sec */
