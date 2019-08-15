#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;

/** Frees an index memory object. */
void dict_mem_index_free(dict_index_t *index); /*!< in: index */
