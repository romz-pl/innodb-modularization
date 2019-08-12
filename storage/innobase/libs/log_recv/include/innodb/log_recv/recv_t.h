#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_recv/recv_data_t.h>
#include <innodb/log_types/mlog_id_t.h>
#include <innodb/log_sn/lsn_t.h>
#include <innodb/lst/lst.h>

/** Stored log record struct */
struct recv_t {
  using Node = UT_LIST_NODE_T(recv_t);

  /** Log record type */
  mlog_id_t type;

  /** Log record body length in bytes */
  ulint len;

  /** Chain of blocks containing the log record body */
  recv_data_t *data;

  /** Start lsn of the log segment written by the mtr which generated
  this log record: NOTE that this is not necessarily the start lsn of
  this log record */
  lsn_t start_lsn;

  /** End lsn of the log segment written by the mtr which generated
  this log record: NOTE that this is not necessarily the end LSN of
  this log record */
  lsn_t end_lsn;

  /** List node, list anchored in recv_addr_t */
  Node rec_list;
};

