#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/commit_node_state.h>
#include <innodb/que_types/que_common_t.h>

/** Commit command node in a query graph */
struct commit_node_t {
  que_common_t common;          /*!< node type: QUE_NODE_COMMIT */
  enum commit_node_state state; /*!< node execution state */
};
