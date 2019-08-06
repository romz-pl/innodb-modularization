#pragma once

#include <innodb/univ/univ.h>



bool fil_space_read_name_and_filepath(space_id_t space_id, char **name,
                                      char **filepath)
    MY_ATTRIBUTE((warn_unused_result));
