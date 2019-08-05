#include <innodb/memory/mem_heap_printf_low.h>

#include <innodb/assert/assert.h>
#include <innodb/error/ut_error.h>

#include <cstring>

/** Helper function for mem_heap_printf.
 @return length of formatted string, including terminating NUL */
ulint mem_heap_printf_low(
    char *buf,          /*!< in/out: buffer to store formatted string
                        in, or NULL to just calculate length */
    const char *format, /*!< in: format string */
    va_list ap)         /*!< in: arguments */
{
  ulint len = 0;

  while (*format) {
    /* Does this format specifier have the 'l' length modifier. */
    ibool is_long = FALSE;

    /* Length of one parameter. */
    size_t plen;

    if (*format++ != '%') {
      /* Non-format character. */

      len++;

      if (buf) {
        *buf++ = *(format - 1);
      }

      continue;
    }

    if (*format == 'l') {
      is_long = TRUE;
      format++;
    }

    switch (*format++) {
      case 's':
        /* string */
        {
          char *s = va_arg(ap, char *);

          /* "%ls" is a non-sensical format specifier. */
          ut_a(!is_long);

          plen = strlen(s);
          len += plen;

          if (buf) {
            memcpy(buf, s, plen);
            buf += plen;
          }
        }

        break;

      case 'u':
        /* unsigned int */
        {
          char tmp[32];
          unsigned long val;

          /* We only support 'long' values for now. */
          ut_a(is_long);

          val = va_arg(ap, unsigned long);

          plen = sprintf(tmp, "%lu", val);
          len += plen;

          if (buf) {
            memcpy(buf, tmp, plen);
            buf += plen;
          }
        }

        break;

      case '%':

        /* "%l%" is a non-sensical format specifier. */
        ut_a(!is_long);

        len++;

        if (buf) {
          *buf++ = '%';
        }

        break;

      default:
        ut_error;
    }
  }

  /* For the NUL character. */
  len++;

  if (buf) {
    *buf = '\0';
  }

  return (len);
}

