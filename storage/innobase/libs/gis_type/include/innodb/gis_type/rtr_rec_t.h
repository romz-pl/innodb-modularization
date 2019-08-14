#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

typedef struct rtr_rec {
  rec_t *r_rec; /*!< matched record */
  bool locked;  /*!< whether the record locked */
} rtr_rec_t;
