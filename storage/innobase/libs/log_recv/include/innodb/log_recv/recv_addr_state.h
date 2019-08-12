#pragma once

#include <innodb/univ/univ.h>

/** States of recv_addr_t */
enum recv_addr_state {

  /** not yet processed */
  RECV_NOT_PROCESSED,

  /** page is being read */
  RECV_BEING_READ,

  /** log records are being applied on the page */
  RECV_BEING_PROCESSED,

  /** log records have been applied on the page */
  RECV_PROCESSED,

  /** log records have been discarded because the tablespace
  does not exist */
  RECV_DISCARDED
};
