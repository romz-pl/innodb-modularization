#pragma once

#include <innodb/univ/univ.h>

/** Compares two thread ids for equality.
@param[in]	lhs	OS thread or thread id
@param[in]	rhs	OS thread or thread id
return true if equal */
#define os_thread_eq(lhs, rhs) ((lhs) == (rhs))
