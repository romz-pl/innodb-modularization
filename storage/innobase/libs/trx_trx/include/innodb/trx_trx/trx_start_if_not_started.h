#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_start_if_not_started_low.h>

struct trx_t;

#ifdef UNIV_DEBUG

#define trx_start_if_not_started(t, rw)    \
  do {                                     \
    (t)->start_line = __LINE__;            \
    (t)->start_file = __FILE__;            \
    trx_start_if_not_started_low((t), rw); \
  } while (false)

#else

#define trx_start_if_not_started(t, rw) trx_start_if_not_started_low((t), rw)

#endif
