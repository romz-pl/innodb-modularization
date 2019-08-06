#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Set the flush observer of a mini-transaction. */
#define mtr_set_flush_observer(m, d) (m)->set_flush_observer((d))
