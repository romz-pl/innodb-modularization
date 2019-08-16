#pragma once

#include <innodb/univ/univ.h>

#include <innodb/cmp/cmp_dtuple_rec_with_match_low.h>
#include <innodb/data_types/dtuple_get_n_fields_cmp.h>

#define cmp_dtuple_rec_with_match(tuple, rec, index, offsets, fields) \
  cmp_dtuple_rec_with_match_low(tuple, rec, index, offsets,           \
                                dtuple_get_n_fields_cmp(tuple), fields)
