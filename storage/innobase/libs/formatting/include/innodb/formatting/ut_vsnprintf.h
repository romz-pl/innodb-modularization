#pragma once

#include <innodb/univ/univ.h>

#ifdef _WIN32
/** A substitute for vsnprintf(3), formatted output conversion into
 a limited buffer. Note: this function DOES NOT return the number of
 characters that would have been printed if the buffer was unlimited because
 VC's _vsnprintf() returns -1 in this case and we would need to call
 _vscprintf() in addition to estimate that but we would need another copy
 of "ap" for that and VC does not provide va_copy(). */
void ut_vsnprintf(char *str,       /*!< out: string */
                  size_t size,     /*!< in: str size */
                  const char *fmt, /*!< in: format */
                  va_list ap);     /*!< in: format values */
#else
/** A wrapper for vsnprintf(3), formatted output conversion into
 a limited buffer. Note: this function DOES NOT return the number of
 characters that would have been printed if the buffer was unlimited because
 VC's _vsnprintf() returns -1 in this case and we would need to call
 _vscprintf() in addition to estimate that but we would need another copy
 of "ap" for that and VC does not provide va_copy(). */
#define ut_vsnprintf(buf, size, fmt, ap) ((void)vsnprintf(buf, size, fmt, ap))
#endif /* _WIN32 */
