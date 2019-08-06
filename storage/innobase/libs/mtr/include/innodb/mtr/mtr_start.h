#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Start a mini-transaction. */
#define mtr_start(m) (m)->start()
