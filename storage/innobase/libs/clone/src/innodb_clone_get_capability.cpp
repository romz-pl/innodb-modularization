#include <innodb/clone/innodb_clone_get_capability.h>

void innodb_clone_get_capability(Ha_clone_flagset &flags) {
  flags.reset();

  flags.set(HA_CLONE_HYBRID);
  flags.set(HA_CLONE_MULTI_TASK);
  flags.set(HA_CLONE_RESTART);
}
