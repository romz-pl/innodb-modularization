#include <innodb/string/innobase_strcasecmp.h>

#include "m_ctype.h"

/** Compares NUL-terminated UTF-8 strings case insensitively.
 @return 0 if a=b, <0 if a<b, >1 if a>b */
int innobase_strcasecmp(const char *a, /*!< in: first string to compare */
                        const char *b) /*!< in: second string to compare */
{
  if (!a) {
    if (!b) {
      return (0);
    } else {
      return (-1);
    }
  } else if (!b) {
    return (1);
  }

  return (my_strcasecmp(system_charset_info, a, b));
}
