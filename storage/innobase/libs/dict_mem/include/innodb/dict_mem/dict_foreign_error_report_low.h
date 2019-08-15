#pragma once

#include <innodb/univ/univ.h>

#include <cstdio>

void dict_foreign_error_report_low(
    FILE *file,       /*!< in: output stream */
    const char *name); /*!< in: table name */
