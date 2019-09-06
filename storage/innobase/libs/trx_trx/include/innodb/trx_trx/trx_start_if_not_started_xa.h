#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_start_if_not_started_xa_low.h>
#include <innodb/trx_trx/trx_t.h>

struct trx_t;

#ifdef UNIV_DEBUG

#define trx_start_if_not_started_xa(t, rw)    \
  do {                                        \
    (t)->start_line = __LINE__;               \
    (t)->start_file = __FILE__;               \
    trx_start_if_not_started_xa_low((t), rw); \
  } while (false)


#else

#define trx_start_if_not_started_xa(t, rw) \
  trx_start_if_not_started_xa_low((t), (rw))

#endif /* UNIV_DEBUG */
