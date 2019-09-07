#pragma once

#include <innodb/univ/univ.h>

/** Rollback node states */
enum roll_node_state {
  ROLL_NODE_NONE = 0, /*!< Unknown state */
  ROLL_NODE_SEND,     /*!< about to send a rollback signal to
                      the transaction */
  ROLL_NODE_WAIT      /*!< rollback signal sent to the
                      transaction, waiting for completion */
};
