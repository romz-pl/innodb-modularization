#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Lock an rw-lock in s-mode. */
#define mtr_s_lock(l, m) (m)->s_lock((l), __FILE__, __LINE__)
