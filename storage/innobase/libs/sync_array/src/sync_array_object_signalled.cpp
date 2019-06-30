#include <innodb/sync_array/sync_array_object_signalled.h>

#include <innodb/sync_array/sg_count.h>

/** Increments the signalled count. */
void sync_array_object_signalled() {
    ++sg_count;
}
