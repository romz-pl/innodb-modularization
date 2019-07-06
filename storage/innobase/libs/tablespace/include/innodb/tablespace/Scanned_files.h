#pragma once

#include <innodb/univ/univ.h>

#include <string>
#include <vector>
#include <utility>

/* uint16_t is the index into Tablespace_dirs::m_dirs */
using Scanned_files = std::vector<std::pair<uint16_t, std::string>>;
