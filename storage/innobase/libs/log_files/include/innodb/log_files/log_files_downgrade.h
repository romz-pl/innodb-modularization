#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Changes format of redo files to previous format version.

@note Note this will work between the two formats 5_7_9 & current because
the only change is the version number */
void log_files_downgrade(log_t &log);
