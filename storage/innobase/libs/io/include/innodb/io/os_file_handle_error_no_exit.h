#pragma once

#include <innodb/univ/univ.h>

bool os_file_handle_error_no_exit(const char *name, const char *operation, bool on_error_silent);
