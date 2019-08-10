#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Dblwr_Block.h>

#include <vector>

/** Vector of doublewrite buffer blocks and their info. */
using Arch_Dblwr_Blocks = std::vector<Arch_Dblwr_Block>;
