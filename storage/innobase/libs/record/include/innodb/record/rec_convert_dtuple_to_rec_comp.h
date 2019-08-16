#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;
struct dfield_t;
struct dtuple_t;

bool rec_convert_dtuple_to_rec_comp(rec_t *rec, const dict_index_t *index,
                                    const dfield_t *fields, ulint n_fields,
                                    const dtuple_t *v_entry, ulint status,
                                    bool temp);
