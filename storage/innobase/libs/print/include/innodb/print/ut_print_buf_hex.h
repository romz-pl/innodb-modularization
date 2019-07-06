#pragma once

#include <innodb/univ/univ.h>

#include <iosfwd>

void ut_print_buf_hex(std::ostream &o, const void *buf, ulint len);
