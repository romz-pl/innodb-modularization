#pragma once

#include <innodb/univ/univ.h>

#include <stdio.h>
#include <iosfwd>

void ut_print_buf(FILE *file, const void *buf, ulint len);

void ut_print_buf(std::ostream &o, const void *buf, ulint len);
