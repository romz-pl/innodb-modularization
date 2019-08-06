#pragma once

#include <innodb/univ/univ.h>

/** Type used for checkpoint numbers (consecutive checkpoints receive
a number which is increased by one). */
typedef uint64_t checkpoint_no_t;
