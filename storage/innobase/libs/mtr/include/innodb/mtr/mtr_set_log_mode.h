#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Change the logging mode of a mini-transaction.
@return	old mode */
#define mtr_set_log_mode(m, d) (m)->set_log_mode((d))
