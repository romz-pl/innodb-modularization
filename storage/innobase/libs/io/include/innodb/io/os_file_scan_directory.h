#pragma once

#include <innodb/univ/univ.h>

/** Callback function type to be implemented by caller. It is called for each
entry in directory.
@param[in]	path	path to the file
@param[in]	name	name of the file */
typedef void (*os_dir_cbk_t)(const char *path, const char *name);


bool os_file_scan_directory(const char *path, os_dir_cbk_t scan_cbk, bool is_delete);

