#pragma once

#include <innodb/univ/univ.h>

bool recv_check_log_header_checksum(const byte *buf);
