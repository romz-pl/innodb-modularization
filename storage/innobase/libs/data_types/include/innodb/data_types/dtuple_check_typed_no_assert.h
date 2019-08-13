#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

ibool dtuple_check_typed_no_assert(const dtuple_t *tuple);
