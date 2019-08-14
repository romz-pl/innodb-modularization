#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>


/* This is to record the record movement between pages. Used for corresponding
lock movement */
typedef struct rtr_rec_move {
  rec_t *old_rec; /*!< record being moved in old page */
  rec_t *new_rec; /*!< new record location */
  bool moved;     /*!< whether lock are moved too */
} rtr_rec_move_t;
