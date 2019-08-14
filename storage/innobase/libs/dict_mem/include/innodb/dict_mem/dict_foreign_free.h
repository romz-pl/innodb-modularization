#pragma once

#include <innodb/univ/univ.h>

struct dict_foreign_t;

void dict_foreign_free(dict_foreign_t *foreign);
