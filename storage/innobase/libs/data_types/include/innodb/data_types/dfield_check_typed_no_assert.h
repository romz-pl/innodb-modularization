#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

ibool dfield_check_typed_no_assert(const dfield_t *field);
