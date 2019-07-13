#pragma once

#include <innodb/univ/univ.h>

char *fil_path_to_space_name(const char *filename)
    MY_ATTRIBUTE((warn_unused_result));
