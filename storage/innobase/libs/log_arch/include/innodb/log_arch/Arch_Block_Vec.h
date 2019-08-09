#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <vector>

class Arch_Block;

/** Vector of page archive in memory blocks */
using Arch_Block_Vec = std::vector<Arch_Block *, ut_allocator<Arch_Block *>>;
