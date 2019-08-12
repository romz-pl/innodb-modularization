#pragma once

#include <innodb/univ/univ.h>

bool log_block_checksum_is_ok(const byte *block);
