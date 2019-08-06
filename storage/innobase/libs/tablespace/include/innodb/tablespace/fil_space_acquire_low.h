#pragma once

#include <innodb/univ/univ.h>



struct fil_space_t;

fil_space_t *fil_space_acquire_low(space_id_t space_id, bool silent);
