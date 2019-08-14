#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/univ/rec_t.h>

#include <stdio.h>

/** Prints an old-style spatial index record. */
void rec_print_mbr_old(FILE *file,       /*!< in: file where to print */
                              const rec_t *rec); /*!< in: physical record */

#endif
