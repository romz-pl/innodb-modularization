#pragma once

#include <innodb/univ/univ.h>


struct fil_space_t;

fil_space_t *fil_space_get(space_id_t space_id);
