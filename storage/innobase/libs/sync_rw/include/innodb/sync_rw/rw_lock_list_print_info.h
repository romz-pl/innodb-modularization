#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifdef UNIV_DEBUG

/** Prints debug info of currently locked rw-locks. */
void rw_lock_list_print_info(FILE *file); /*!< in: file where to print */

#endif /* UNIV_DEBUG */
#endif /* UNIV_LIBRARY */
