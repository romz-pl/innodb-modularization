#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/check_trx_state.h>

/**
Assert that the transaction is in the trx_sys_t::rw_trx_list */
#define assert_trx_in_rw_list(t)                         \
  do {                                                   \
    ut_ad(!(t)->read_only);                              \
    ut_ad((t)->in_rw_trx_list ==                         \
          !((t)->read_only || !(t)->rsegs.m_redo.rseg)); \
    check_trx_state(t);                                  \
  } while (0)
