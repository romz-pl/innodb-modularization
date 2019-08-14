#pragma once

#include <innodb/univ/univ.h>

#include <innodb/gis_type/node_seq_t.h>
#include <innodb/sync_mutex/ib_mutex_t.h>

/* Node Sequence Number and mutex protects it. */
typedef struct rtree_ssn {
  ib_mutex_t mutex;  /*!< mutex protect the seq num */
  node_seq_t seq_no; /*!< the SSN (node sequence number) */
} rtr_ssn_t;
