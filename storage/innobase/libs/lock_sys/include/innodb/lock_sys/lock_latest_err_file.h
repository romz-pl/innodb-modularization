#pragma once

#include <innodb/univ/univ.h>

#include <cstdio>

/** Only created if !srv_read_only_mode */
extern FILE *lock_latest_err_file;
