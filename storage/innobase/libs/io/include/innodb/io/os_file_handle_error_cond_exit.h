#pragma once

#include <innodb/univ/univ.h>

bool os_file_handle_error_cond_exit(const char *name, const char *operation, bool should_exit, bool on_error_silent);
