#include <innodb/math/ut_2_exp.h>

/** Calculates 2 to power n.
@param[in]	n	power of 2
@return 2 to power n */
uint32_t ut_2_exp(uint32_t n) {
    return (1 << n);
}
