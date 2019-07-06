#pragma once

#include <innodb/univ/univ.h>

ulint ut_raw_to_hex(const void *raw, ulint raw_size, char *hex, ulint hex_size);
