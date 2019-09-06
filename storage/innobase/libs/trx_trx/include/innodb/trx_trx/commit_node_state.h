#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Commit node states */
enum commit_node_state {
  COMMIT_NODE_SEND = 1, /*!< about to send a commit signal to
                        the transaction */
  COMMIT_NODE_WAIT      /*!< commit signal sent to the transaction,
                        waiting for completion */
};
