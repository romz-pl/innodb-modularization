#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/lsn_t.h>

/** When the oldest dirty page age exceeds this value, we start
an asynchronous preflush of dirty pages. This function does not
have side-effects, it only reads and returns the limit value.
@return age of dirty page at which async. preflush is started */
lsn_t log_get_max_modified_age_async();

#endif
