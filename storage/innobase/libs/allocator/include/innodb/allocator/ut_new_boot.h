#pragma once

#include <innodb/univ/univ.h>

/** Setup the internal objects needed for UT_NEW() to operate.
This must be called before the first call to UT_NEW(). */
void ut_new_boot();
