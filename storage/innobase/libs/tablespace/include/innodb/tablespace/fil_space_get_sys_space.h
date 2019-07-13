#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/fil_space_t.h>

/** @return the system tablespace instance */
#define fil_space_get_sys_space() (fil_space_t::s_sys_space)
