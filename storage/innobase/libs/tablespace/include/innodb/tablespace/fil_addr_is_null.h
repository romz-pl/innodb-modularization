#pragma once

#include <innodb/univ/univ.h>

struct fil_addr_t;

bool fil_addr_is_null(const fil_addr_t &addr)
    MY_ATTRIBUTE((warn_unused_result));

