#include <innodb/sync_latch/sync_file_created_register.h>
#include <innodb/sync_latch/create_tracker.h>

/** Register a latch, called when it is created
@param[in]	ptr		Latch instance that was created
@param[in]	filename	Filename where it was created
@param[in]	line		Line number in filename */
void sync_file_created_register(const void *ptr, const char *filename,
                                uint16_t line) {
  create_tracker->register_latch(ptr, filename, line);
}
