#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_types/lock_mode.h>

const char *lock_mode_string(enum lock_mode mode);
