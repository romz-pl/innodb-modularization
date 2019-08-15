#pragma once

#include <innodb/univ/univ.h>

/** Return a SDI Index id for given SDI copy
@return index_id for SDI copy */
UNIV_INLINE
space_index_t dict_sdi_get_index_id() {
    return (IB_UINT64_MAX);
}
