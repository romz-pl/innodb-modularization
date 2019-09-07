#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_rsegsf_t.h>

struct mtr_t;

trx_rsegsf_t *trx_rsegsf_get(space_id_t space_id, mtr_t *mtr) ;
