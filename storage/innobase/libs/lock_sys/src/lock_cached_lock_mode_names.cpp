#include <innodb/lock_sys/lock_cached_lock_mode_names.h>

/** Used by lock_get_mode_str to cache results. Strings pointed by these
pointers might be in use by performance schema and thus can not be freed
until the very end */
std::unordered_map<uint, const char *> lock_cached_lock_mode_names;
