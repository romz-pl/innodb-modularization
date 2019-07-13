#pragma once

#include <innodb/univ/univ.h>

struct fil_space_t;

void fil_space_update_name(fil_space_t *space, const char *name);
