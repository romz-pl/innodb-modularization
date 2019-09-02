#pragma once

#include <innodb/univ/univ.h>

#include <unordered_map>

/** Used by lock_get_mode_str to cache results. Strings pointed by these
pointers might be in use by performance schema and thus can not be freed
until the very end */
extern std::unordered_map<uint, const char *> lock_cached_lock_mode_names;
