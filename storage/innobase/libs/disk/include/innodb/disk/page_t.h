#pragma once

#include <innodb/univ/univ.h>

/** Eliminates a name collision on HP-UX */
#define page_t ib_page_t

/** Type of the index page */
typedef byte page_t;
