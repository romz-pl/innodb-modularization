#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct commit_node_t;

/** Creates a commit command node struct.
 @return own: commit node struct */
commit_node_t *trx_commit_node_create(
    mem_heap_t *heap); /*!< in: mem heap where created */
