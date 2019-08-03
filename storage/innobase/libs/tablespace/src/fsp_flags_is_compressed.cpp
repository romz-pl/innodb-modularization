#include <innodb/tablespace/fsp_flags_is_compressed.h>

#include <innodb/tablespace/consts.h>

/** Determine if the tablespace is compressed from tablespace flags.
@param[in]	flags	Tablespace flags
@return true if compressed, false if not compressed */
bool fsp_flags_is_compressed(uint32_t flags) {
  return (FSP_FLAGS_GET_ZIP_SSIZE(flags) != 0);
}
