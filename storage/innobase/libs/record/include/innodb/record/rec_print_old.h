#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#include <stdio.h>

/** Prints an old-style physical record. */
void rec_print_old(FILE *file,        /*!< in: file where to print */
                   const rec_t *rec); /*!< in: physical record */
