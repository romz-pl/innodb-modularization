#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Lock a tablespace in x-mode. */
#define mtr_x_lock_space(s, m) (m)->x_lock_space((s), __FILE__, __LINE__)
