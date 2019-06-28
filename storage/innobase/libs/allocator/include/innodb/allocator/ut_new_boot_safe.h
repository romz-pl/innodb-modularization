#pragma once

#include <innodb/allocator/ut_new_boot_safe.h>

/** Setup the internal objects needed for UT_NEW() to operate.
This must be called before the first call to UT_NEW(). This
version of function might be called several times and it will
simply skip all calls except the first one, during which the
initialization will happen. */
void ut_new_boot_safe();
