#include <innodb/allocator/ut_new_boot_safe.h>

#include <innodb/allocator/ut_new_boot.h>

void ut_new_boot_safe() {
  static bool ut_new_boot_called = false;

  if (!ut_new_boot_called) {
    ut_new_boot();
    ut_new_boot_called = true;
  }
}
