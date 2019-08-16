#include <innodb/cmp/cmp_decimal.h>


/** Compare two DATA_DECIMAL (MYSQL_TYPE_DECIMAL) fields.
TODO: Remove this function. Everything should use MYSQL_TYPE_NEWDECIMAL.
@param[in]	a		data field
@param[in]	a_length	length of a, in bytes (not UNIV_SQL_NULL)
@param[in]	b		data field
@param[in]	b_length	length of b, in bytes (not UNIV_SQL_NULL)
@param[in]	is_asc		true=ascending, false=descending order
@return positive, 0, negative, if a is greater, equal, less than b,
respectively */
UNIV_COLD int cmp_decimal(const byte *a, unsigned int a_length,
                                 const byte *b, unsigned int b_length,
                                 bool is_asc) {
  int swap_flag = is_asc ? 1 : -1;

  /* Remove preceding spaces */
  for (; a_length && *a == ' '; a++, a_length--) {
  }
  for (; b_length && *b == ' '; b++, b_length--) {
  }

  if (*a == '-') {
    swap_flag = -swap_flag;

    if (*b != '-') {
      return (swap_flag);
    }

    a++;
    b++;
    a_length--;
    b_length--;
  } else {
    if (*b == '-') {
      return (swap_flag);
    }
  }

  while (a_length > 0 && (*a == '+' || *a == '0')) {
    a++;
    a_length--;
  }

  while (b_length > 0 && (*b == '+' || *b == '0')) {
    b++;
    b_length--;
  }

  if (a_length != b_length) {
    if (a_length < b_length) {
      return (-swap_flag);
    }

    return (swap_flag);
  }

  while (a_length > 0 && *a == *b) {
    a++;
    b++;
    a_length--;
  }

  if (a_length == 0) {
    return (0);
  }

  if (*a <= *b) {
    swap_flag = -swap_flag;
  }

  return (swap_flag);
}
