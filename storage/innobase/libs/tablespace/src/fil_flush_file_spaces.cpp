#include <innodb/tablespace/fil_flush_file_spaces.h>

#include <innodb/tablespace/fil_system.h>

/** Flush to disk the writes in file spaces of the given type
possibly cached by the OS.
@param[in]     purpose FIL_TYPE_TABLESPACE or FIL_TYPE_LOG, can be ORred */
void fil_flush_file_spaces(uint8_t purpose) {
  fil_system->flush_file_spaces(purpose);
}
