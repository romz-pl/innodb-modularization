#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/gis_type/node_seq_t.h>

struct btr_pcur_t;


/* RTree internal non-leaf Nodes to be searched, from root to leaf */
typedef struct node_visit {
  page_no_t page_no;  /*!< the page number */
  node_seq_t seq_no;  /*!< the SSN (split sequence number */
  ulint level;        /*!< the page's index level */
  page_no_t child_no; /*!< child page num if for parent
                      recording */
  btr_pcur_t *cursor; /*!< cursor structure if we positioned
                      FIXME: there is no need to use whole
                      btr_pcur_t, just the position related
                      members */
  double mbr_inc;     /*!< whether this node needs to be
                      enlarged for insertion */
} node_visit_t;
