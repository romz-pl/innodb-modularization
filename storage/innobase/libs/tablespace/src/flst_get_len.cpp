#include <innodb/tablespace/flst_get_len.h>

#include <innodb/tablespace/FLST_LEN.h>
#include <innodb/machine/data.h>

/** Get the length of a list.
@param[in]	base	base node
@return length */
ulint flst_get_len(const flst_base_node_t *base) {
  return (mach_read_from_4(base + FLST_LEN));
}
