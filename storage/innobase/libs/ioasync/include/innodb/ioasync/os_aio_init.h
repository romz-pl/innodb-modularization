#pragma once

#include <innodb/univ/univ.h>

bool os_aio_init(ulint n_readers, ulint n_writers, ulint n_slots_sync);
