#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

/** Initialize Page and Log archiver system
@return error code */
dberr_t arch_init();
