#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** The currently used undo space IDs for an undo space number
along with a boolean showing whether the undo space number is in use. */
struct space_id_account {
  space_id_t space_id;
  bool in_use;
};

}
