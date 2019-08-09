#include <innodb/log_arch/Page_Arch_Client_Ctx.h>

#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/logger/info.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_types/log_get_checkpoint_lsn.h>
#include <innodb/log_types/log_get_lsn.h>


#ifdef UNIV_DEBUG
void Page_Arch_Client_Ctx::print() {
  DBUG_PRINT("page_archiver", ("CLIENT INFO"));
  DBUG_PRINT("page_archiver", ("Transient Client - %u", !m_is_durable));
  DBUG_PRINT("page_archiver", ("Start LSN - %" PRIu64 "", m_start_lsn));
  DBUG_PRINT("page_archiver", ("Stop LSN - %" PRIu64 "", m_stop_lsn));
  DBUG_PRINT("page_archiver",
             ("Last Reset LSN - %" PRIu64 "", m_last_reset_lsn));
  DBUG_PRINT("page_archiver", ("Start pos - %" PRIu64 ", %u",
                               m_start_pos.m_block_num, m_start_pos.m_offset));
  DBUG_PRINT("page_archiver", ("Stop pos - %" PRIu64 ", %u\n",
                               m_stop_pos.m_block_num, m_stop_pos.m_offset));
}
#endif

int Page_Arch_Client_Ctx::start(bool recovery, uint64_t *start_id) {
  bool reset = false;
  int err = 0;
  std::ostringstream msg;

  arch_client_mutex_enter();

  switch (m_state) {
    case ARCH_CLIENT_STATE_STOPPED:
      if (!m_is_durable) {
        ib::error() << "Client needs to release its resources";
        return (ER_PAGE_TRACKING_NOT_STARTED);
      }
      DBUG_PRINT("page_archiver", ("Archiver in progress"));
      DBUG_PRINT("page_archiver", ("[->] Starting page archiving."));
      break;

    case ARCH_CLIENT_STATE_INIT:
      DBUG_PRINT("page_archiver", ("Archiver in progress"));
      DBUG_PRINT("page_archiver", ("[->] Starting page archiving."));
      break;

    case ARCH_CLIENT_STATE_STARTED:
      DBUG_PRINT("page_archiver", ("[->] Resetting page archiving."));
      ut_ad(m_group != nullptr);
      reset = true;
      break;

    default:
      ut_ad(false);
  }

  /* Start archiving. */
  err = arch_page_sys->start(&m_group, &m_last_reset_lsn, &m_start_pos,
                             m_is_durable, reset, recovery);

  if (err != 0) {
    arch_client_mutex_exit();
    return (err);
  }

  if (!reset) {
    m_start_lsn = m_last_reset_lsn;
  }

  if (start_id != nullptr) {
    *start_id = m_last_reset_lsn;
  }

  if (!is_active()) {
    m_state = ARCH_CLIENT_STATE_STARTED;
  }

  arch_client_mutex_exit();

  if (!m_is_durable) {
    ib::info(ER_IB_MSG_20) << "Clone Start PAGE ARCH : start LSN : "
                           << m_start_lsn << ", checkpoint LSN : "
                           << log_get_checkpoint_lsn(*log_sys);
  }

  return (err);
}

int Page_Arch_Client_Ctx::init_during_recovery(Arch_Group *group,
                                               lsn_t last_lsn) {
  /* Initialise the sys client */
  m_state = ARCH_CLIENT_STATE_STARTED;
  m_group = group;
  m_start_lsn = group->get_begin_lsn();
  m_last_reset_lsn = last_lsn;
  m_start_pos.init();

  /* Start page archiving. */
  int error = start(true, nullptr);

  ut_d(print());

  return (error);
}

int Page_Arch_Client_Ctx::stop(lsn_t *stop_id) {
  arch_client_mutex_enter();

  if (!is_active()) {
    arch_client_mutex_exit();
    ib::error(ER_PAGE_TRACKING_NOT_STARTED);
    return (ER_PAGE_TRACKING_NOT_STARTED);
  }

  ut_ad(m_group != nullptr);

  /* Stop archiving. */
  auto err =
      arch_page_sys->stop(m_group, &m_stop_lsn, &m_stop_pos, m_is_durable);

  if (err != 0) {
    arch_client_mutex_exit();
    return (err);
  }

  ut_d(print());

  m_state = ARCH_CLIENT_STATE_STOPPED;

  if (stop_id != nullptr) {
    *stop_id = m_stop_lsn;
  }

  arch_client_mutex_exit();

  ib::info(ER_IB_MSG_21) << "Clone Stop  PAGE ARCH : end   LSN : " << m_stop_lsn
                         << ", log sys LSN : " << log_get_lsn(*log_sys);

  return (err);
}

int Page_Arch_Client_Ctx::get_pages(Page_Arch_Cbk *cbk_func, void *cbk_ctx,
                                    byte *buff, uint buf_len) {
  int err = 0;
  uint num_pages;
  uint read_len;

  arch_client_mutex_enter();

  ut_ad(m_state == ARCH_CLIENT_STATE_STOPPED);

  auto cur_pos = m_start_pos;

  while (true) {
    ut_ad(cur_pos.m_block_num <= m_stop_pos.m_block_num);

    /* Check if last block */
    if (cur_pos.m_block_num >= m_stop_pos.m_block_num) {
      if (cur_pos.m_offset > m_stop_pos.m_offset) {
        ut_ad(false);
        my_error(ER_INTERNAL_ERROR, MYF(0), "Wrong Archiver page offset");
        err = ER_INTERNAL_ERROR;
        break;
      }

      read_len = m_stop_pos.m_offset - cur_pos.m_offset;

      if (read_len == 0) {
        break;
      }

    } else {
      if (cur_pos.m_offset > ARCH_PAGE_BLK_SIZE) {
        ut_ad(false);
        my_error(ER_INTERNAL_ERROR, MYF(0), "Wrong Archiver page offset");
        err = ER_INTERNAL_ERROR;
        break;
      }

      read_len = ARCH_PAGE_BLK_SIZE - cur_pos.m_offset;

      /* Move to next block. */
      if (read_len == 0) {
        cur_pos.set_next();
        continue;
      }
    }

    if (read_len > buf_len) {
      read_len = buf_len;
    }

    err = m_group->read_data(cur_pos, buff, read_len);

    if (err != 0) {
      break;
    }

    cur_pos.m_offset += read_len;
    num_pages = read_len / ARCH_BLK_PAGE_ID_SIZE;

    err = cbk_func(cbk_ctx, buff, num_pages);

    if (err != 0) {
      break;
    }
  }

  arch_client_mutex_exit();

  return (err);
}

void Page_Arch_Client_Ctx::release() {
  arch_client_mutex_enter();

  switch (m_state) {
    case ARCH_CLIENT_STATE_INIT:
      arch_client_mutex_exit();
      return;

    case ARCH_CLIENT_STATE_STARTED:
      arch_client_mutex_exit();
      stop(nullptr);
      break;

    case ARCH_CLIENT_STATE_STOPPED:
      break;

    default:
      ut_ad(false);
  }

  ut_ad(m_group != nullptr);

  arch_page_sys->release(m_group, m_is_durable, m_start_pos);

  m_state = ARCH_CLIENT_STATE_INIT;
  m_group = nullptr;
  m_start_lsn = LSN_MAX;
  m_stop_lsn = LSN_MAX;
  m_last_reset_lsn = LSN_MAX;
  m_start_pos.init();
  m_stop_pos.init();

  arch_client_mutex_exit();
}
