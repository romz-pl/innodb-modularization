#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_HOTBACKUP

bool os_file_close_no_error_handling(os_file_t file);

#endif /* UNIV_HOTBACKUP */
