#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to set the next record offset field of a
new-style record. */
void rec_set_next_offs_new(rec_t *rec, ulint next);
