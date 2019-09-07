#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct roll_node_t;

/** Creates a rollback command node struct.
 @return own: rollback node struct */
roll_node_t *roll_node_create(
    mem_heap_t *heap); /*!< in: mem heap where created */
