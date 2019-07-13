#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <stdio.h>

FILE *os_file_create_tmpfile(const char *path);

#endif /* !UNIV_HOTBACKUP */

