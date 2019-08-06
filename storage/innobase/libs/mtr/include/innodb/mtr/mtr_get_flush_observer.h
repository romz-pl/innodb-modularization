#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Get the flush observer of a mini-transaction.
@return flush observer object */
#define mtr_get_flush_observer(m) (m)->get_flush_observer()
