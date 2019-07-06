#include <innodb/string/ut_str3cat.h>

#include <innodb/allocator/ut_malloc_nokey.h>

#include <string.h>

/** Concatenate 3 strings.*/
char *ut_str3cat(
    /* out, own: concatenated string, must be
    freed with ut_free() */
    const char *s1, /* in: string 1 */
    const char *s2, /* in: string 2 */
    const char *s3) /* in: string 3 */
{
  char *s;
  ulint s1_len = strlen(s1);
  ulint s2_len = strlen(s2);
  ulint s3_len = strlen(s3);

  s = static_cast<char *>(ut_malloc_nokey(s1_len + s2_len + s3_len + 1));

  memcpy(s, s1, s1_len);
  memcpy(s + s1_len, s2, s2_len);
  memcpy(s + s1_len + s2_len, s3, s3_len);

  s[s1_len + s2_len + s3_len] = '\0';

  return (s);
}
