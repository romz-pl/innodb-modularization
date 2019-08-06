#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Commit a mini-transaction. */
#define mtr_commit(m) (m)->commit()
