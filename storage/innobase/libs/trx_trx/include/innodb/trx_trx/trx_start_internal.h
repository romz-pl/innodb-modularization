#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_start_internal_low.h>

struct trx_t;

#ifdef UNIV_DEBUG

#define trx_start_internal(t)    \
  do {                           \
    (t)->start_line = __LINE__;  \
    (t)->start_file = __FILE__;  \
    trx_start_internal_low((t)); \
  } while (false)

#else

#define trx_start_internal(t) trx_start_internal_low((t))

#endif
