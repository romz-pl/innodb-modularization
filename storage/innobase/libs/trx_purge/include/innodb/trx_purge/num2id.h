#pragma once

#include <innodb/univ/univ.h>

namespace undo {

space_id_t num2id(space_id_t space_num, size_t ndx);
space_id_t num2id(space_id_t space_num);

}
