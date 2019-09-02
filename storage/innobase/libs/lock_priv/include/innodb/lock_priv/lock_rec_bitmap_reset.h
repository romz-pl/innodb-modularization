#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

void lock_rec_bitmap_reset(lock_t *lock);
