#include <innodb/sync_latch/sync_file_created_deregister.h>
#include <innodb/sync_latch/create_tracker.h>

/** Deregister a latch, called when it is destroyed
@param[in]	ptr		Latch to be destroyed */
void sync_file_created_deregister(const void *ptr) {
  create_tracker->deregister_latch(ptr);
}
