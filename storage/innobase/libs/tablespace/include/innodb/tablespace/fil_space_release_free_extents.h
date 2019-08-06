#pragma once

#include <innodb/univ/univ.h>



void fil_space_release_free_extents(space_id_t space_id, ulint n_reserved);
