#include <innodb/memory/validate_no_mans_land.h>

#ifdef UNIV_DEBUG
UNIV_INLINE
void validate_no_mans_land(byte *no_mans_land_begin,
                           byte mem_no_mans_land_byte) {
  UNIV_MEM_VALID(no_mans_land_begin, MEM_NO_MANS_LAND);
  for (byte *no_mans_land_it = no_mans_land_begin;
       no_mans_land_it < no_mans_land_begin + MEM_NO_MANS_LAND;
       ++no_mans_land_it) {
    ut_a(*no_mans_land_it == mem_no_mans_land_byte);
  }
}
#endif
