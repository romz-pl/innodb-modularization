#pragma once

#include <innodb/assert/assert.h>

//
// Abort execution.
//
#define ut_error ut_dbg_assertion_failed(0, __FILE__, (ulint)__LINE__)
