#pragma once

#include <innodb/univ/univ.h>

/** Remove files related to page and log archiving.
@param[in]	file_path	path to the file
@param[in]	file_name	name of the file */
void arch_remove_file(const char *file_path, const char *file_name);
