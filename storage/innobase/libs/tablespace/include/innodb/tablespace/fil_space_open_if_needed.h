#pragma once

#include <innodb/univ/univ.h>

struct fil_space_t;

void fil_space_open_if_needed(fil_space_t *space);
