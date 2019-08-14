#pragma once

#include <innodb/univ/univ.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/univ/rec_t.h>

#include <stdio.h>

/** Prints a spatial index record. */
void rec_print_mbr_rec(
    FILE *file,            /*!< in: file where to print */
    const rec_t *rec,      /*!< in: physical record */
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */

#endif
