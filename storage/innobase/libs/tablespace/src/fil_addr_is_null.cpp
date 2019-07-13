#include <innodb/tablespace/fil_addr_is_null.h>

#include <innodb/tablespace/fil_addr_t.h>

/** Returns true if file address is undefined.
@param[in]	addr		Address
@return true if undefined */
bool fil_addr_is_null(const fil_addr_t &addr) {
  return (addr.page == FIL_NULL);
}
