#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/formatting/get_first_format.h>
#include <type_traits>

namespace ib {

/** Verifies that the `fmt` format string does not require any arguments
@param[in]   fmt  The format string
@return true if and only if there is no format specifier inside `fmt` which
requires passing an argument */
inline bool verify_fmt_match(const char *fmt) {
  return (get_first_format(fmt) == nullptr);
}

/** Verifies that the `fmt` format string contains format specifiers which match
the type and order of the arguments
@param[in]  fmt   The format string
@param[in]  head  The first argument
@param[in]  tail  Others (perhaps none) arguments
@return true if and only if the format specifiers found in `fmt` correspond to
types of head, tail...
*/
template <typename Head, typename... Tail>
bool verify_fmt_match(const char *fmt, Head &&head, Tail &&... tail) {
  using H =
      typename std::remove_cv<typename std::remove_reference<Head>::type>::type;
  const char *pos = get_first_format(fmt);
  if (pos == nullptr) {
    return (false);
  }
  /* We currently only handle :
  %[-0-9.*]*(d|ld|lld|u|lu|llu|zu|zx|zd|s|x|i|f|c|X|p|lx|llx|lf)
  Feel free to extend the parser, if you need something more, as the parser is
  not intended to be any stricter than real printf-format parser, and if it does
  not handle something, it is only to keep the code simpler. */
  const std::string skipable("-+ #0123456789.*");

  pos++;
  while (*pos != '\0' && skipable.find_first_of(*pos) != std::string::npos) {
    pos++;
  }
  if (*pos == '\0') {
    return (false);
  }
  bool is_ok = true;
  if (pos[0] == 'l') {
    if (pos[1] == 'l') {
      if (pos[2] == 'd') {
        is_ok = std::is_same<H, long long int>::value;
      } else if (pos[2] == 'u' || pos[2] == 'x') {
        is_ok = std::is_same<H, unsigned long long int>::value;
      } else {
        is_ok = false;
      }
    } else if (pos[1] == 'd') {
      is_ok = std::is_same<H, long int>::value;
    } else if (pos[1] == 'u') {
      is_ok = std::is_same<H, unsigned long int>::value;
    } else if (pos[1] == 'x') {
      is_ok = std::is_same<H, unsigned long int>::value;
    } else if (pos[1] == 'f') {
      is_ok = std::is_same<H, double>::value;
    } else {
      is_ok = false;
    }
  } else if (pos[0] == 'd') {
    is_ok = std::is_same<H, int>::value;
  } else if (pos[0] == 'u') {
    is_ok = std::is_same<H, unsigned int>::value;
  } else if (pos[0] == 'x') {
    is_ok = std::is_same<H, unsigned int>::value;
  } else if (pos[0] == 'X') {
    is_ok = std::is_same<H, unsigned int>::value;
  } else if (pos[0] == 'i') {
    is_ok = std::is_same<H, int>::value;
  } else if (pos[0] == 'f') {
    is_ok = std::is_same<H, float>::value;
  } else if (pos[0] == 'c') {
    is_ok = std::is_same<H, char>::value;
  } else if (pos[0] == 'p') {
    is_ok = std::is_pointer<H>::value;
  } else if (pos[0] == 's') {
    is_ok = (std::is_same<H, char *>::value ||
             std::is_same<H, char const *>::value ||
             (std::is_array<H>::value &&
              std::is_same<typename std::remove_cv<
                               typename std::remove_extent<H>::type>::type,
                           char>::value));
  } else if (pos[0] == 'z') {
    if (pos[1] == 'u') {
      is_ok = std::is_same<H, size_t>::value;
    } else if (pos[1] == 'x') {
      is_ok = std::is_same<H, size_t>::value;
    } else if (pos[1] == 'd') {
      is_ok = std::is_same<H, ssize_t>::value;
    } else {
      is_ok = false;
    }
  } else {
    is_ok = false;
  }
  return (is_ok && verify_fmt_match(pos + 1, std::forward<Tail>(tail)...));
}

}
#endif /* UNIV_DEBUG */
