#include <innodb/log_arch/arch_remove_file.h>

#include <innodb/assert/assert.h>
#include <innodb/io/os_file_delete.h>
#include <innodb/io/pfs.h>
#include <innodb/log_arch/flags.h>

#include <cstring>

void arch_remove_file(const char *file_path, const char *file_name) {
  char path[MAX_ARCH_PAGE_FILE_NAME_LEN];

  ut_ad(MAX_ARCH_LOG_FILE_NAME_LEN <= MAX_ARCH_PAGE_FILE_NAME_LEN);
  ut_ad(strlen(file_path) + 1 + strlen(file_name) <
        MAX_ARCH_PAGE_FILE_NAME_LEN);

  /* Remove only LOG and PAGE archival files. */
  if (0 != strncmp(file_name, ARCH_LOG_FILE, strlen(ARCH_LOG_FILE)) &&
      0 != strncmp(file_name, ARCH_PAGE_FILE, strlen(ARCH_PAGE_FILE)) &&
      0 != strncmp(file_name, ARCH_PAGE_GROUP_DURABLE_FILE_NAME,
                   strlen(ARCH_PAGE_GROUP_DURABLE_FILE_NAME))) {
    return;
  }

  snprintf(path, sizeof(path), "%s%c%s", file_path, OS_PATH_SEPARATOR,
           file_name);

#ifdef UNIV_DEBUG
  os_file_type_t type;
  bool exists = false;

  os_file_status(path, &exists, &type);
  ut_a(exists);
  ut_a(type == OS_FILE_TYPE_FILE);
#endif /* UNIV_DEBUG */

  os_file_delete(innodb_arch_file_key, path);
}
