#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/Block.h>

#include <vector>

/** For storing the allocated blocks */
typedef std::vector<Block> Blocks;
