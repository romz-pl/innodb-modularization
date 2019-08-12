#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_recv/recv_t.h>
#include <innodb/disk/page_no_t.h>
#include <innodb/log_recv/recv_addr_state.h>
#include <innodb/lst/lst.h>

/** Hashed page file address struct */
struct recv_addr_t {
  using List = UT_LIST_BASE_NODE_T(recv_t);

  /** recovery state of the page */
  recv_addr_state state;

  /** Space ID */
  space_id_t space;

  /** Page number */
  page_no_t page_no;

  /** List of log records for this page */
  List rec_list;
};
