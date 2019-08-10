#pragma once

#include <innodb/univ/univ.h>

#include <map>


/** Map for current block number for unfinished chunks. Used during
restart from incomplete clone operation. */
using Chunk_Map = std::map<uint32_t, uint32_t>;
