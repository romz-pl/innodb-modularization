#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_rsegsf_t.h>

struct mtr_t;

page_no_t trx_rsegsf_get_page_no(trx_rsegsf_t *rsegs_header, ulint slot,
                                        mtr_t *mtr);
