#pragma once

#include <innodb/univ/univ.h>

struct fil_space_t;

void fil_space_set_flags(fil_space_t *space, uint32_t flags);
