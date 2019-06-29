#include <innodb/sync_latch/create_tracker.h>

/** Track latch creation location. For reducing the size of the latches */
CreateTracker *create_tracker = nullptr;
