#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <stdio.h>

/** Prints all pending IO
@param[in]	file	file where to print */
void os_aio_print_pending_io(FILE *file);

#endif /* UNIV_DEBUG */
