#pragma once

#include <innodb/univ/univ.h>

/* Type of (un)encryption operation in progress for Tablespace. */
enum encryption_op_type {
    ENCRYPTION = 1,
    UNENCRYPTION = 2,
    NONE
};
