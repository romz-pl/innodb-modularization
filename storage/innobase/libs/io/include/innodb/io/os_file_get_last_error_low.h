#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/error_codes.h>

ulint os_file_get_last_error_low(bool report_all_errors, bool on_error_silent);
