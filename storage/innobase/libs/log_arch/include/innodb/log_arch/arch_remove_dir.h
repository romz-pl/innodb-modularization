#pragma once

#include <innodb/univ/univ.h>

/** Remove group directory and the files related to page and log archiving.
@param[in]	dir_path	path to the directory
@param[in]	dir_name	directory name */
void arch_remove_dir(const char *dir_path, const char *dir_name);
