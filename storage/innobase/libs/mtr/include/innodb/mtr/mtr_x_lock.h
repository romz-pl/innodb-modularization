#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Lock an rw-lock in x-mode. */
#define mtr_x_lock(l, m) (m)->x_lock((l), __FILE__, __LINE__)
