#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

ulint lock_rec_lock_fold(const lock_t *lock);
