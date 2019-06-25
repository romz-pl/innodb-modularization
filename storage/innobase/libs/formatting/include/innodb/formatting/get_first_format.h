#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <string.h>

namespace ib {

/** Finds the first format specifier in `fmt` format string
@param[in]   fmt  The format string
@return Either the longest suffix of `fmt` which starts with format specifier,
or `nullptr` if could not find any format specifier inside `fmt`.
*/
inline const char *get_first_format(const char *fmt) {
  const char *pos = strchr(fmt, '%');
  if (pos != nullptr && pos[1] == '%') {
    return (get_first_format(pos + 2));
  }
  return (pos);
}

}

#endif
