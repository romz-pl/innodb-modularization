#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/record/rec_t.h>

#include <stdio.h>

void rec_print_comp(
    FILE *file,           /*!< in: file where to print */
    const rec_t *rec,     /*!< in: physical record */
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */


#endif
