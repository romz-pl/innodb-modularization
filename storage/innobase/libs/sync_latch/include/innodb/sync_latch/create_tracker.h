#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_latch/CreateTracker.h>

/** Track latch creation location. For reducing the size of the latches */
extern CreateTracker *create_tracker;
