#pragma once

#include <innodb/univ/univ.h>

space_id_t trx_rseg_id_to_space_id(ulint id, bool is_temp);
