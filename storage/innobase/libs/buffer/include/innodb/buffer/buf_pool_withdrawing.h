#pragma once

#include <innodb/univ/univ.h>

/** true when withdrawing buffer pool pages might cause page relocation */
extern volatile bool buf_pool_withdrawing;
