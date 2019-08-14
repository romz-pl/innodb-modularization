#pragma once

#include <innodb/univ/univ.h>

#include <innodb/gis_type/flags.h>
#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/univ/page_cur_mode_t.h>
#include <innodb/gis_type/rtr_mbr_t.h>

struct rtr_node_path_t;
struct matched_rec_t;
struct buf_block_t;
struct que_thr_t;
struct btr_cur_t;
struct dict_index_t;
struct dtuple_t;

/** Vectors holding the matching internal pages/nodes and leaf records */
typedef struct rtr_info {
  rtr_node_path_t *path; /*!< vector holding matching pages */
  rtr_node_path_t *parent_path;
  /*!< vector holding parent pages during
  search */
  matched_rec_t *matches; /*!< struct holding matching leaf records */
  ib_mutex_t rtr_path_mutex;
  /*!< mutex protect the "path" vector */
  buf_block_t *tree_blocks[RTR_MAX_LEVELS + RTR_LEAF_LATCH_NUM];
  /*!< tracking pages that would be locked
  at leaf level, for future free */
  ulint tree_savepoints[RTR_MAX_LEVELS + RTR_LEAF_LATCH_NUM];
  /*!< savepoint used to release latches/blocks
  on each level and leaf level */
  rtr_mbr_t mbr;       /*!< the search MBR */
  que_thr_t *thr;      /*!< the search thread */
  mem_heap_t *heap;    /*!< memory heap */
  btr_cur_t *cursor;   /*!< cursor used for search */
  dict_index_t *index; /*!< index it is searching */
  bool need_prdt_lock;
  /*!< whether we will need predicate lock
  the tree */
  bool need_page_lock;
  /*!< whether we will need predicate page lock
  the tree */
  bool allocated; /*!< whether this structure is allocate or
                on stack */
  bool mbr_adj;   /*!< whether mbr will need to be enlarged
                  for an insertion operation */
  bool fd_del;    /*!< found deleted row */
  const dtuple_t *search_tuple;
  /*!< search tuple being used */
  page_cur_mode_t search_mode;
  /*!< current search mode */

  /* TODO: This is for a temporary fix, will be removed later */
  bool *is_dup;
  /*!< whether the current rec is a duplicate record. */
} rtr_info_t;
