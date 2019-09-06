#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_start_internal_read_only.h>

struct trx_t;


#ifdef UNIV_DEBUG

#define trx_start_internal_read_only(t)  \
  do {                                   \
    (t)->start_line = __LINE__;          \
    (t)->start_file = __FILE__;          \
    trx_start_internal_read_only_low(t); \
  } while (false)

#else

#define trx_start_internal_read_only(t) trx_start_internal_read_only_low(t)

#endif /* UNIV_DEBUG */
