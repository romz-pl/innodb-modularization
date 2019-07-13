#include <innodb/tablespace/fil_get_compression.h>

#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_space_t.h>

/** Get the compression algorithm for a tablespace.
@param[in]	space_id	Space ID to check
@return the compression algorithm */
Compression::Type fil_get_compression(space_id_t space_id) {
  fil_space_t *space = fil_space_get(space_id);

  return (space == nullptr ? Compression::NONE : space->compression_type);
}
