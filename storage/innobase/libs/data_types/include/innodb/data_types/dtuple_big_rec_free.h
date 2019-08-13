#pragma once

#include <innodb/univ/univ.h>

struct big_rec_t;

/** Frees the memory in a big rec vector. */
void dtuple_big_rec_free(big_rec_t *vector); /*!< in, own: big rec vector; it is
                                     freed in this function */
