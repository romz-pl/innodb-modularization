#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifdef UNIV_DEBUG

/** Prints info of a debug struct. */
void rw_lock_debug_print(FILE *f,                      /*!< in: output stream */
                         const rw_lock_debug_t *info); /*!< in: debug struct */

#endif /* UNIV_DEBUG */
#endif /* UNIV_LIBRARY */
