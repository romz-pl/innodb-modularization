#pragma once

#include <innodb/univ/univ.h>

#include <string>

bool fil_check_path(const std::string &path) MY_ATTRIBUTE((warn_unused_result));
