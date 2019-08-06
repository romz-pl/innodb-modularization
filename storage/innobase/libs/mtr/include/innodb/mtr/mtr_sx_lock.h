#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Lock an rw-lock in sx-mode. */
#define mtr_sx_lock(l, m) (m)->sx_lock((l), __FILE__, __LINE__)
