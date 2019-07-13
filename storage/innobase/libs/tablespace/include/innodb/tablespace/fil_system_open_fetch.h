#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

#include <string>

std::string fil_system_open_fetch(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
