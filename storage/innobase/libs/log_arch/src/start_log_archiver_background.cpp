#include <innodb/log_arch/start_log_archiver_background.h>

#include <innodb/io/os_file_create_directory.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/log_archiver_is_active.h>
#include <innodb/log_arch/log_archiver_thread.h>
#include <innodb/log_arch/pfs.h>
#include <innodb/thread/os_thread_create.h>

#include "my_sys.h"

int start_log_archiver_background() {
  bool ret;
  char errbuf[MYSYS_STRERROR_SIZE];

  ret = os_file_create_directory(ARCH_DIR, false);

  if (ret) {
    log_archiver_is_active = true;

    os_thread_create(log_archiver_thread_key, log_archiver_thread);
  } else {
    my_error(ER_CANT_CREATE_FILE, MYF(0), ARCH_DIR, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));

    return (ER_CANT_CREATE_FILE);
  }

  return (0);
}
