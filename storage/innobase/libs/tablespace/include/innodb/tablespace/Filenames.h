#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <string>
#include <vector>

using Filenames = std::vector<std::string, ut_allocator<std::string>>;
