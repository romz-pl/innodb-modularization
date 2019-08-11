#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct log_t;

bool log_write_to_file_requests_are_frequent(uint64_t interval);
bool log_write_to_file_requests_are_frequent(const log_t &log);

#endif
