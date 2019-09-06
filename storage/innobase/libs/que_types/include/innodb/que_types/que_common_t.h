#pragma once

#include <innodb/univ/univ.h>

#include <innodb/data_types/dfield_t.h>
#include <innodb/que_types/que_node_t.h>

/* Common struct at the beginning of each query graph node; the name of this
substruct must be 'common' */
struct que_common_t {
  ulint type;          /*!< query node type */
  que_node_t *parent;  /*!< back pointer to parent node, or NULL */
  que_node_t *brother; /* pointer to a possible brother node */
  dfield_t val;        /*!< evaluated value for an expression */
  ulint val_buf_size;
  /* buffer size for the evaluated value data,
  if the buffer has been allocated dynamically:
  if this field is != 0, and the node is a
  symbol node or a function node, then we
  have to free the data field in val
  explicitly */
};
