#include <innodb/log_arch/arch_free.h>

#include <innodb/log_arch/arch_log_sys.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/log_arch/log_archiver_thread_event.h>
#include <innodb/log_arch/page_archiver_thread_event.h>
#include <innodb/sync_event/os_event_destroy.h>

/** Free Page and Log archiver system */
void arch_free() {
  if (arch_log_sys != nullptr) {
    UT_DELETE(arch_log_sys);
    arch_log_sys = nullptr;

    os_event_destroy(log_archiver_thread_event);
  }

  if (arch_page_sys != nullptr) {
    UT_DELETE(arch_page_sys);
    arch_page_sys = nullptr;

    os_event_destroy(page_archiver_thread_event);
  }
}
