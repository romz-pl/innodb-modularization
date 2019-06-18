#pragma once

/** Debug assertion. Does nothing unless UNIV_DEBUG is defined. */

#ifdef UNIV_DEBUG
#define ut_d(EXPR) EXPR
#else
#define ut_d(EXPR)
#endif
