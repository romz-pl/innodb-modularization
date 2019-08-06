#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP



struct rw_lock_t;
rw_lock_t *fil_space_get_latch(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result));

#endif
