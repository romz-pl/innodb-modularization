#pragma once

#include <innodb/univ/univ.h>

/* Space address data type; this is intended to be used when
addresses accurate to a byte are stored in file pages. If the page part
of the address is FIL_NULL, the address is considered undefined. */

/** 'type' definition in C: an address stored in a file page is a
string of bytes */
using fil_faddr_t = byte;



