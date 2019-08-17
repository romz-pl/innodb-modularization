#include <innodb/page/page_zip_fail.h>


/* Enable some extra debugging output.  This code can be enabled
independently of any UNIV_ debugging conditions. */
#if defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG

#include <stdarg.h>

MY_ATTRIBUTE((format(printf, 1, 2)))
/** Report a failure to decompress or compress.
 @return number of characters printed */
int page_zip_fail_func(const char *fmt, /*!< in: printf(3) format string */
                       ...) /*!< in: arguments corresponding to fmt */
{
  int res;
  va_list ap;

  ut_print_timestamp(stderr);
  fputs("  InnoDB: ", stderr);
  va_start(ap, fmt);
  res = vfprintf(stderr, fmt, ap);
  va_end(ap);

  return (res);
}
/** Wrapper for page_zip_fail_func()
@param fmt_args in: printf(3) format string and arguments */
#define page_zip_fail(fmt_args) page_zip_fail_func fmt_args

#endif                          /* UNIV_DEBUG || UNIV_ZIP_DEBUG */
