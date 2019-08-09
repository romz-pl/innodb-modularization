#include <innodb/log_arch/arch_remove_dir.h>

#include <innodb/assert/assert.h>
#include <innodb/io/os_file_scan_directory.h>
#include <innodb/log_arch/arch_remove_file.h>
#include <innodb/log_arch/flags.h>

#include <cstring>

void arch_remove_dir(const char *dir_path, const char *dir_name) {
  char path[MAX_ARCH_DIR_NAME_LEN];

  ut_ad(sizeof(ARCH_LOG_DIR) <= sizeof(ARCH_PAGE_DIR));
  ut_ad(strlen(dir_path) + 1 + strlen(dir_name) + 1 < sizeof(path));

  /* Remove only LOG and PAGE archival directories. */
  if (0 != strncmp(dir_name, ARCH_LOG_DIR, strlen(ARCH_LOG_DIR)) &&
      0 != strncmp(dir_name, ARCH_PAGE_DIR, strlen(ARCH_PAGE_DIR))) {
    return;
  }

  snprintf(path, sizeof(path), "%s%c%s", dir_path, OS_PATH_SEPARATOR, dir_name);

#ifdef UNIV_DEBUG
  os_file_type_t type;
  bool exists = false;

  os_file_status(path, &exists, &type);
  ut_a(exists);
  ut_a(type == OS_FILE_TYPE_DIR);
#endif /* UNIV_DEBUG */

  os_file_scan_directory(path, arch_remove_file, true);
}
