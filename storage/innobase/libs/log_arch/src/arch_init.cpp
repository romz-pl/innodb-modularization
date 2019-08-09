#include <innodb/log_arch/arch_init.h>

#include <innodb/allocator/UT_NEW.h>
#include <innodb/log_arch/Arch_Page_Sys.h>
#include <innodb/log_arch/arch_log_sys.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/log_arch/log_archiver_thread_event.h>
#include <innodb/log_arch/page_archiver_thread_event.h>

/** Initialize Page and Log archiver system
@return error code */
dberr_t arch_init() {
  if (arch_log_sys == nullptr) {
    arch_log_sys = UT_NEW(Arch_Log_Sys(), mem_key_archive);

    if (arch_log_sys == nullptr) {
      return (DB_OUT_OF_MEMORY);
    }

    log_archiver_thread_event = os_event_create(0);
  }

  if (arch_page_sys == nullptr) {
    arch_page_sys = UT_NEW(Arch_Page_Sys(), mem_key_archive);

    if (arch_page_sys == nullptr) {
      return (DB_OUT_OF_MEMORY);
    }

    page_archiver_thread_event = os_event_create(0);
  }

  arch_page_sys->recover();

  return (DB_SUCCESS);
}
