#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

dberr_t fil_open_for_business(bool read_only_mode)
    MY_ATTRIBUTE((warn_unused_result));
