#pragma once

#include <innodb/univ/univ.h>

bool fil_delete_file(const char *path) MY_ATTRIBUTE((warn_unused_result));
