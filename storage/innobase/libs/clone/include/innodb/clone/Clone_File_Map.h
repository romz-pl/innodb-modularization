#pragma once

#include <innodb/univ/univ.h>

#include <map>

/** Map type for mapping space ID to clone file index */
using Clone_File_Map = std::map<space_id_t, uint>;
