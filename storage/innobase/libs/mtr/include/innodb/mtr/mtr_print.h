#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Print info of an mtr handle. */
#define mtr_print(m) (m)->print()
