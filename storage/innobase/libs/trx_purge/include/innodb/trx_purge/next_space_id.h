#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Given a valid undo space_id or SPACE_UNKNOWN, return the next space_id
for the given space number.
@param[in]  space_id   undo tablespace ID
@param[in]  space_num  undo tablespace number
@return the next tablespace ID to use */
space_id_t next_space_id(space_id_t space_id, space_id_t space_num);

/** Given a valid undo space_id, return the next space_id for that
space number.
@param[in]  space_id  undo tablespace ID
@return the next tablespace ID to use */
space_id_t next_space_id(space_id_t space_id);

}
