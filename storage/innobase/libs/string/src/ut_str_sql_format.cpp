#include <innodb/string/ut_str_sql_format.h>

/** Adds single quotes to the start and end of string and escapes any quotes
 by doubling them. Returns the number of bytes that were written to "buf"
 (including the terminating NUL). If buf_size is too small then the
 trailing bytes from "str" are discarded.
 @return number of bytes that were written */
ulint ut_str_sql_format(const char *str, /*!< in: string */
                        ulint str_len,   /*!< in: string length in bytes */
                        char *buf,       /*!< out: output buffer */
                        ulint buf_size)  /*!< in: output buffer size
                                         in bytes */
{
  ulint str_i;
  ulint buf_i;

  buf_i = 0;

  switch (buf_size) {
    case 3:

      if (str_len == 0) {
        buf[buf_i] = '\'';
        buf_i++;
        buf[buf_i] = '\'';
        buf_i++;
      }
      /* FALLTHROUGH */
    case 2:
    case 1:

      buf[buf_i] = '\0';
      buf_i++;
      /* FALLTHROUGH */
    case 0:

      return (buf_i);
  }

  /* buf_size >= 4 */

  buf[0] = '\'';
  buf_i = 1;

  for (str_i = 0; str_i < str_len; str_i++) {
    char ch;

    if (buf_size - buf_i == 2) {
      break;
    }

    ch = str[str_i];

    switch (ch) {
      case '\0':

        if (buf_size - buf_i < 4) {
          goto func_exit;
        }
        buf[buf_i] = '\\';
        buf_i++;
        buf[buf_i] = '0';
        buf_i++;
        break;
      case '\'':
      case '\\':

        if (buf_size - buf_i < 4) {
          goto func_exit;
        }
        buf[buf_i] = ch;
        buf_i++;
        /* FALLTHROUGH */
      default:

        buf[buf_i] = ch;
        buf_i++;
    }
  }

func_exit:

  buf[buf_i] = '\'';
  buf_i++;
  buf[buf_i] = '\0';
  buf_i++;

  return (buf_i);
}
