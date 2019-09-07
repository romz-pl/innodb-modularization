#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** List of currently used undo space IDs for each undo space number
along with a boolean showing whether the undo space number is in use. */
extern struct space_id_account *space_id_bank;

}
