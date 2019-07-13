#include <innodb/tablespace/fil_close.h>

#include <innodb/tablespace/fil_system.h>

/** Closes the tablespace memory cache. */
void fil_close() {
  if (fil_system == nullptr) {
    return;
  }

  UT_DELETE(fil_system);

  fil_system = nullptr;
}
