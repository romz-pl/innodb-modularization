#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

ulint lock_rec_get_rec_not_gap(const lock_t *lock);
