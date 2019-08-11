#pragma once

#include <innodb/univ/univ.h>

/** Initializes the log system. Note that the log system is not ready
for user writes after this call is finished. It should be followed by
a call to log_start. Also, log background threads need to be started
manually using log_start_background_threads afterwards.

Hence the proper order of calls looks like this:
        - log_sys_init(),
        - log_start(),
        - log_start_background_threads().

@param[in]	n_files		number of log files
@param[in]	file_size	size of each log file in bytes
@param[in]	space_id	space id of the file space with log files */
bool log_sys_init(uint32_t n_files, uint64_t file_size, space_id_t space_id);
