#include <innodb/tablespace/fil_space_get_page_size.h>

#include <innodb/tablespace/fil_space_get_flags.h>
#include <innodb/page/univ_page_size.h>

/** Returns the page size of the space and whether it is compressed or not.
The tablespace must be cached in the memory cache.
@param[in]	space_id	Tablespace ID
@param[out]	found		true if tablespace was found
@return page size */
const page_size_t fil_space_get_page_size(space_id_t space_id, bool *found) {
  const uint32_t flags = fil_space_get_flags(space_id);

  if (flags == UINT32_UNDEFINED) {
    *found = false;
    return (univ_page_size);
  }

  *found = true;

  return (page_size_t(flags));
}
