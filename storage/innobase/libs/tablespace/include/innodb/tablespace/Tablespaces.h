#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

#include "sql/dd/object_id.h"

#include <tuple>
#include <vector>

/** Used for collecting the data in boot_tablespaces() */
namespace dd_fil {

enum {
  /** DD Object ID */
  OBJECT_ID,

  /** InnoDB tablspace ID */
  SPACE_ID,

  /** DD/InnoDB tablespace name */
  SPACE_NAME,

  /** Path in DD tablespace */
  OLD_PATH,

  /** Path where it was found during the scan. */
  NEW_PATH
};

using Moved = std::tuple<dd::Object_id, space_id_t, std::string, std::string,
                         std::string>;

using Tablespaces = std::vector<Moved>;
}  // namespace dd_fil
