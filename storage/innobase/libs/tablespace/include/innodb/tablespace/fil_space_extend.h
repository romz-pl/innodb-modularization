#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_no_t.h>

struct fil_space_t;

bool fil_space_extend(fil_space_t *space, page_no_t size)
    MY_ATTRIBUTE((warn_unused_result));
