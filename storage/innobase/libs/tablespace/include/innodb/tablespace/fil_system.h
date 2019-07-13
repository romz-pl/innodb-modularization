#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/Fil_system.h>

/** The tablespace memory cache. This variable is nullptr before the module is
initialized. */
extern Fil_system *fil_system;
