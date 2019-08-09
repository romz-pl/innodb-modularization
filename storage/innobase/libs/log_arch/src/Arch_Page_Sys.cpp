#include <innodb/log_arch/Arch_Page_Sys.h>

#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/log_arch/Page_Arch_Client_Ctx.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_types/log_get_checkpoint_lsn.h>

Arch_Page_Sys::Arch_Page_Sys() {
  mutex_create(LATCH_ID_PAGE_ARCH, &m_mutex);
  mutex_create(LATCH_ID_PAGE_ARCH_OPER, &m_oper_mutex);

  m_ctx = UT_NEW(Page_Arch_Client_Ctx(true), mem_key_archive);

  DBUG_EXECUTE_IF("page_archiver_simulate_more_archived_files",
                  ARCH_PAGE_FILE_CAPACITY = 8;
                  ARCH_PAGE_FILE_DATA_CAPACITY =
                      ARCH_PAGE_FILE_CAPACITY - ARCH_PAGE_FILE_NUM_RESET_PAGE;);
}

Arch_Page_Sys::~Arch_Page_Sys() {
  ut_ad(m_state == ARCH_STATE_INIT || m_state == ARCH_STATE_ABORT);
  ut_ad(m_current_group == nullptr);

  for (auto group : m_group_list) {
    UT_DELETE(group);
  }

  Arch_Group::shutdown();

  m_data.clean();

  UT_DELETE(m_ctx);
  mutex_free(&m_mutex);
  mutex_free(&m_oper_mutex);
}

void Arch_Page_Sys::post_recovery_init() {
  if (!is_active()) {
    return;
  }

  arch_oper_mutex_enter();
  m_latest_stop_lsn = log_get_checkpoint_lsn(*log_sys);
  auto cur_block = m_data.get_block(&m_write_pos, ARCH_DATA_BLOCK);
  update_stop_info(cur_block);
  arch_oper_mutex_exit();
}

