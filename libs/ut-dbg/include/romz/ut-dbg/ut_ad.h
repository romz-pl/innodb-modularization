#pragma once

#include <romz/ut-dbg/ut_a.h>

/** Debug assertion. Does nothing unless UNIV_DEBUG is defined. */

#ifdef UNIV_DEBUG
#define ut_ad(EXPR) ut_a(EXPR)
#else
#define ut_ad(EXPR)
#endif
