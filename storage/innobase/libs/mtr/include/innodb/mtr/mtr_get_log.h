#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Return the log object of a mini-transaction buffer.
@return	log */
#define mtr_get_log(m) (m)->get_log()
