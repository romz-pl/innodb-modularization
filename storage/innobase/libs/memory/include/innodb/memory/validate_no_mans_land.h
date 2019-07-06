#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG
void validate_no_mans_land(byte *no_mans_land_begin,
                           byte mem_no_mans_land_byte);
#endif
