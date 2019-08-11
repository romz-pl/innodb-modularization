#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

/** @return true iff log_free_check should be executed. */
bool log_needs_free_check();

#endif

