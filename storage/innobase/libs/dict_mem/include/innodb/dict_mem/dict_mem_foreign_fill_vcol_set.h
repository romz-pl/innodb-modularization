#pragma once

#include <innodb/univ/univ.h>

struct dict_foreign_t;

/** Fills the dependent virtual columns in a set.
Reason for being dependent are
1) FK can be present on base column of virtual columns
2) FK can be present on column which is a part of virtual index
@param[in,out]	foreign	foreign key information. */
void dict_mem_foreign_fill_vcol_set(dict_foreign_t *foreign);
