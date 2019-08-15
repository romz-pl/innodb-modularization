#pragma once

#include <innodb/univ/univ.h>

struct dict_foreign_t;

/** Removes a foreign constraint struct from the dictionary cache. */
void dict_foreign_remove_from_cache(
    dict_foreign_t *foreign); /*!< in, own: foreign constraint */
