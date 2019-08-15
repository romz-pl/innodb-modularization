#pragma once

#include <innodb/univ/univ.h>

struct dict_foreign_t;

/** Creates and initializes a foreign constraint memory object.
 @return own: foreign constraint struct */
dict_foreign_t *dict_mem_foreign_create(void);
