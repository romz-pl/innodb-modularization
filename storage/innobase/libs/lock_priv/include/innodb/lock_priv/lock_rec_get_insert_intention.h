#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

ulint lock_rec_get_insert_intention(const lock_t *lock);
