#include <innodb/log_arch/page_archiver_thread.h>

#include <innodb/log_arch/Arch_Group.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/log_arch/page_archiver_is_active.h>
#include <innodb/log_arch/page_archiver_thread_event.h>
#include <innodb/logger/info.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_wait.h>

/** Archiver background thread */
void page_archiver_thread() {
  bool page_abort = false;
  bool page_wait = false;

  Arch_Group::init_dblwr_file_ctx(
      ARCH_DBLWR_DIR, ARCH_DBLWR_FILE, ARCH_DBLWR_NUM_FILES,
      static_cast<uint64_t>(ARCH_PAGE_BLK_SIZE) * ARCH_DBLWR_FILE_CAPACITY);

  while (true) {
    if (!page_abort) {
      /* Archive in memory data blocks to disk. */
      page_abort = arch_page_sys->archive(&page_wait);

      if (page_abort) {
        ib::info(ER_IB_MSG_14) << "Exiting Page Archiver";
        break;
      }
    }

    if (page_wait) {
      /* Nothing to archive. Wait until next trigger. */
      os_event_wait(page_archiver_thread_event);
      os_event_reset(page_archiver_thread_event);
    }
  }

  page_archiver_is_active = false;
}
