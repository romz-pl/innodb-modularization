#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/tablespace/space_id_t.h>

void fil_space_set_imported(space_id_t space_id);

#endif
