#pragma once

#include <innodb/univ/univ.h>

#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dtuple_t.h>

/* Estimate the number of bytes that are going to be allocated when
creating a new dtuple_t object */
#define DTUPLE_EST_ALLOC(n_fields) \
  (sizeof(dtuple_t) + (n_fields) * sizeof(dfield_t))
