#pragma once

#include <innodb/univ/univ.h>


#ifndef UNIV_HOTBACKUP

#include <vector>

struct row_prebuilt_t;

/** A vector to collect prebuilt from different readers working on the same
temp table */
typedef std::vector<row_prebuilt_t *> temp_prebuilt_vec;

#endif /* !UNIV_HOTBACKUP */
