#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/Compression.h>


Compression::Type fil_get_compression(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
