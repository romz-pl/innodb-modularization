#pragma once

#include <innodb/univ/univ.h>

/** the clock is incremented every time a pointer to a page may become
obsolete */
extern volatile ulint buf_withdraw_clock;
