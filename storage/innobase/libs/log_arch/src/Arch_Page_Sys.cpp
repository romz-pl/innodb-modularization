#include <innodb/log_arch/Arch_Page_Sys.h>

#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/log_arch/Page_Arch_Client_Ctx.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_types/log_get_checkpoint_lsn.h>
#include <innodb/log_arch/Recv.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/Dir_Walker.h>
#include <innodb/monitor/MONITOR_INC.h>
#include <innodb/log_arch/Page_Wait_Flush_Archiver_Cbk.h>
#include <innodb/logger/warn.h>
#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/log_arch/page_archiver_thread_event.h>
#include <innodb/buffer/buf_page_t.h>
#include <innodb/buffer/buf_pool_t.h>
#include <innodb/buffer/srv_buf_pool_instances.h>
#include <innodb/buffer/buf_pool_from_array.h>
#include <innodb/buffer/buf_flush_list_mutex_enter.h>
#include <innodb/buffer/buf_flush_list_mutex_exit.h>
#include <innodb/tablespace/fsp_is_system_temporary.h>
// #include <innodb/log_redo/log_buffer_flush_order_lag.h>
#include <innodb/buffer/buf_page_get_io_fix_unlocked.h>
#include <innodb/log_types/log_buffer_x_lock_exit.h>
#include <innodb/log_types/log_buffer_x_lock_enter.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/log_arch/start_page_archiver_background.h>

void log_request_checkpoint(log_t &log, bool sync);
lsn_t log_buffer_flush_order_lag(const log_t &log);
bool wait_flush_archiver(Page_Wait_Flush_Archiver_Cbk cbk_func);

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

dberr_t Arch_Page_Sys::recover() {
  DBUG_PRINT("page_archiver", ("Crash Recovery"));

  Recv arch_recv(ARCH_DIR);
  dberr_t err;

  err = arch_recv.init();

  if (!arch_recv.scan_group()) {
    DBUG_PRINT("page_archiver", ("No group information available"));
    return (DB_SUCCESS);
  }

  err = arch_recv.fill_info(this);

  if (err != DB_SUCCESS) {
    ib::error() << "Page archiver system's recovery failed";
    return (DB_OUT_OF_MEMORY);
  }

  return (err);
}

dberr_t Arch_Page_Sys::Recv::init() {
  dberr_t err;

  err = m_dblwr_ctx.init(
      ARCH_DBLWR_DIR, ARCH_DBLWR_FILE, ARCH_DBLWR_NUM_FILES,
      static_cast<uint64_t>(ARCH_PAGE_BLK_SIZE) * ARCH_DBLWR_FILE_CAPACITY);

  if (err != DB_SUCCESS) {
    return (err);
  }

  err = m_dblwr_ctx.read_blocks();

  return (err);
}

#ifdef UNIV_DEBUG
void Arch_Page_Sys::Recv::print() {
  for (auto group : m_dir_group_info_map) {
    DBUG_PRINT("page_archiver",
               ("Group : %s\t%u", group.first.c_str(), group.second.m_active));
  }
}
#endif

void Arch_Page_Sys::Recv::read_group_dirs(const std::string file_path) {
  if (file_path.find(ARCH_PAGE_DIR) == std::string::npos) {
    return;
  }

  Arch_Recv_Group_Info info;
  m_dir_group_info_map.insert(
      std::pair<std::string, Arch_Recv_Group_Info>(file_path, info));
}

void Arch_Page_Sys::Recv::read_group_files(const std::string dir_path,
                                           const std::string file_path) {
  if (file_path.find(ARCH_PAGE_FILE) == std::string::npos &&
      file_path.find(ARCH_PAGE_GROUP_ACTIVE_FILE_NAME) == std::string::npos &&
      file_path.find(ARCH_PAGE_GROUP_DURABLE_FILE_NAME) == std::string::npos) {
    return;
  }

  Arch_Recv_Group_Info &info = m_dir_group_info_map[dir_path];

  if (file_path.find(ARCH_PAGE_GROUP_ACTIVE_FILE_NAME) != std::string::npos) {
    info.m_active = true;
    return;
  }

  if (file_path.find(ARCH_PAGE_GROUP_DURABLE_FILE_NAME) != std::string::npos) {
    info.m_durable = true;
    return;
  }

  info.m_num_files += 1;

  auto found = file_path.find(ARCH_PAGE_FILE);
  ut_ad(found != std::string::npos);
  uint file_index = 0;

  /* Fetch start index. */
  try {
    file_index = static_cast<uint>(
        std::stoi(file_path.substr(found + strlen(ARCH_PAGE_FILE))));
  } catch (const std::exception &) {
    ut_ad(0);
    ib::error() << "Invalid archived file name format. The archived file"
                << " is supposed to have the format " << ARCH_PAGE_FILE
                << "+ [0-9]*.";
    return;
  }

  if (info.m_file_start_index > file_index) {
    info.m_file_start_index = file_index;
  }
}

bool Arch_Page_Sys::Recv::scan_group() {
  os_file_type_t type;
  bool exists;
  bool success;

  success = os_file_status(m_arch_dir_name.c_str(), &exists, &type);

  if (!success || !exists || type != OS_FILE_TYPE_DIR) {
    return (false);
  }

  Dir_Walker::walk(m_arch_dir_name, false, [&](const std::string file_path) {
    read_group_dirs(file_path);
  });

  if (m_dir_group_info_map.size() == 0) {
    return (false);
  }

  for (auto it : m_dir_group_info_map) {
    Dir_Walker::walk(it.first, true, [&](const std::string file_path) {
      read_group_files(it.first, file_path);
    });
  }

  ut_d(print());

  return (true);
}

dberr_t Arch_Page_Sys::Recv::fill_info(Arch_Page_Sys *page_sys) {
  uint num_active = 0;
  dberr_t err = DB_SUCCESS;
  bool new_empty_file = false;

  for (auto info = m_dir_group_info_map.begin();
       info != m_dir_group_info_map.end(); ++info) {
    auto group_info = info->second;
    auto dir_name = info->first;

    auto pos = dir_name.find(ARCH_PAGE_DIR);
    lsn_t start_lsn = static_cast<lsn_t>(
        std::stoull(dir_name.substr(pos + strlen(ARCH_PAGE_DIR))));

    Arch_Group *group = UT_NEW(
        Arch_Group(start_lsn, ARCH_PAGE_FILE_HDR_SIZE, page_sys->get_mutex()),
        mem_key_archive);

    Arch_Page_Pos write_pos;
    Arch_Page_Pos reset_pos;

    err = group->recover(&group_info, new_empty_file, &m_dblwr_ctx, write_pos,
                         reset_pos);

    if (err != DB_SUCCESS) {
      UT_DELETE(group);
      return (err);
    }

    if (group_info.m_num_files == 0) {
      UT_DELETE(group);
      continue;
    }

    page_sys->m_group_list.push_back(group);

    if (group_info.m_active) {
      /* Group was active at the time of shutdown/crash, so
      we need to start page archiving */

      page_sys->m_write_pos = write_pos;
      page_sys->m_reset_pos = reset_pos;

      ++num_active;
      int error = page_sys->start_during_recovery(group, new_empty_file);

      if (error != 0) {
        return (DB_ERROR);
      }
    }
  }

  /* There can be only one active group at a time. */
  ut_ad(num_active <= 1);

  return (DB_SUCCESS);
}


void Arch_Page_Sys::flush_at_checkpoint(lsn_t checkpoint_lsn) {
  arch_oper_mutex_enter();

  if (!is_active()) {
    arch_oper_mutex_exit();
    return;
  }

  lsn_t end_lsn = m_current_group->get_end_lsn();

  if (m_write_pos.m_offset == ARCH_PAGE_BLK_HEADER_LENGTH) {
    arch_oper_mutex_exit();
    return;
  }

  Arch_Page_Pos request_flush_pos;

  if (end_lsn == LSN_MAX) {
    Arch_Block *cur_block = m_data.get_block(&m_write_pos, ARCH_DATA_BLOCK);

    ut_ad(cur_block->get_state() == ARCH_BLOCK_ACTIVE);

    m_latest_stop_lsn = checkpoint_lsn;
    update_stop_info(cur_block);

    if (cur_block->get_oldest_lsn() != LSN_MAX &&
        cur_block->get_oldest_lsn() <= checkpoint_lsn) {
      /* If the oldest modified page in the block added since the last
      checkpoint was modified before the checkpoint_lsn then the block needs to
      be flushed*/

      request_flush_pos = m_write_pos;
    } else {
      /* Wait for blocks that are not active to be flushed. */

      if (m_write_pos.m_block_num == 0) {
        arch_oper_mutex_exit();
        return;
      }

      request_flush_pos.init();
      request_flush_pos.m_block_num = m_write_pos.m_block_num - 1;
    }

    if (request_flush_pos < m_flush_pos) {
      arch_oper_mutex_exit();
      return;
    }

    if (m_request_flush_pos < request_flush_pos) {
      m_request_flush_pos = request_flush_pos;
    }

  } else {
    request_flush_pos = m_current_group->get_stop_pos();
    m_request_flush_pos = request_flush_pos;
  }

  if (request_flush_pos.m_block_num == m_write_pos.m_block_num) {
    MONITOR_INC(MONITOR_PAGE_TRACK_CHECKPOINT_PARTIAL_FLUSH_REQUEST);
  }

  /* We need to ensure that blocks are flushed until request_flush_pos */
  auto cbk = [&] { return (request_flush_pos < m_flush_pos ? false : true); };

  bool success = wait_flush_archiver(cbk);

  if (!success) {
    ib::warn() << "Unable to flush. Page archiving data"
               << " may be corrupt in case of a crash";
  }

  arch_oper_mutex_exit();
}

/** Check and add page ID to archived data.
Check for duplicate page.
@param[in]	bpage		page to track
@param[in]	track_lsn	LSN when tracking started
@param[in]	frame_lsn	current LSN of the page
@param[in]	force		if true, add page ID without check */
void Arch_Page_Sys::track_page(buf_page_t *bpage, lsn_t track_lsn,
                               lsn_t frame_lsn, bool force) {
  Arch_Block *cur_blk;
  uint count = 0;

  if (!force) {
    /* If the frame LSN is bigger than track LSN, it
    is already added to tracking list. */
    if (frame_lsn > track_lsn) {
      return;
    }
  }

  /* We need to track this page. */
  arch_oper_mutex_enter();

  while (true) {
    if (m_state != ARCH_STATE_ACTIVE) {
      break;
    }

    /* Can possibly loop only two times. */
    if (count >= 2) {
      if (srv_shutdown_state != SRV_SHUTDOWN_NONE) {
        arch_oper_mutex_exit();
        return;
      }

      ut_ad(false);
      ib::warn(ER_IB_MSG_23) << "Fail to add page for tracking."
                             << " Space ID: " << bpage->id.space();

      m_state = ARCH_STATE_ABORT;
      arch_oper_mutex_exit();
      return;
    }

    cur_blk = m_data.get_block(&m_write_pos, ARCH_DATA_BLOCK);

    if (cur_blk->get_state() == ARCH_BLOCK_ACTIVE) {
      if (cur_blk->add_page(bpage, &m_write_pos)) {
        /* page added successfully. */
        break;
      }

      /* Current block is full. Move to next block. */
      cur_blk->end_write();

      m_write_pos.set_next();

      /* Writing to a new file so move to the next reset block. */
      if (m_write_pos.m_block_num % ARCH_PAGE_FILE_DATA_CAPACITY == 0) {
        Arch_Block *reset_block =
            m_data.get_block(&m_reset_pos, ARCH_RESET_BLOCK);
        reset_block->end_write();

        m_reset_pos.set_next();
      }

      os_event_set(page_archiver_thread_event);

      ++count;
      continue;

    } else if (cur_blk->get_state() == ARCH_BLOCK_INIT ||
               cur_blk->get_state() == ARCH_BLOCK_FLUSHED) {
      ut_ad(m_write_pos.m_offset == ARCH_PAGE_BLK_HEADER_LENGTH);

      cur_blk->begin_write(m_write_pos);

      if (!cur_blk->add_page(bpage, &m_write_pos)) {
        /* Should always succeed. */
        ut_ad(false);
      }

      /* page added successfully. */
      break;

    } else {
      bool success;

      ut_a(cur_blk->get_state() == ARCH_BLOCK_READY_TO_FLUSH);

      auto cbk = std::bind(&Arch_Block::is_flushable, *cur_blk);

      /* Might release operation mutex temporarily. Need to
      loop again verifying the state. */
      success = wait_flush_archiver(cbk);
      count = success ? 0 : 2;

      continue;
    }
  }
  arch_oper_mutex_exit();
}

/** Get page IDs from a specific position.
Caller must ensure that read_len doesn't exceed the block.
@param[in]	read_pos	position in archived data
@param[in]	read_len	amount of data to read
@param[out]	read_buff	buffer to return the page IDs.
                                Caller must allocate the buffer.
@return true if we could successfully read the block. */
bool Arch_Page_Sys::get_pages(Arch_Group *group, Arch_Page_Pos *read_pos,
                              uint read_len, byte *read_buff) {
  Arch_Block *read_blk;
  bool success;

  arch_oper_mutex_enter();

  if (group != m_current_group) {
    arch_oper_mutex_exit();
    return (false);
  }

  /* Get the block to read from. */
  read_blk = m_data.get_block(read_pos, ARCH_DATA_BLOCK);

  read_blk->update_block_header(LSN_MAX, LSN_MAX);

  /* Read from the block. */
  success = read_blk->get_data(read_pos, read_len, read_buff);

  arch_oper_mutex_exit();

  return (success);
}

int Arch_Page_Sys::get_pages(MYSQL_THD thd, Page_Track_Callback cbk_func,
                             void *cbk_ctx, lsn_t &start_id, lsn_t &stop_id,
                             byte *buf, uint buf_len) {
  DBUG_PRINT("page_archiver", ("Fetch pages"));

  arch_mutex_enter();

  /** 1. Get appropriate LSN range. */
  Arch_Group *group = nullptr;

  int error = fetch_group_within_lsn_range(start_id, stop_id, &group);

  DBUG_PRINT("page_archiver", ("Start id: %" PRIu64 ", stop id: %" PRIu64 "",
                               start_id, stop_id));

  if (error != 0) {
    arch_mutex_exit();
    return (error);
  }

  ut_ad(group != nullptr);

  /** 2. Get block position from where to start. */

  Arch_Page_Pos start_pos;
  Arch_Point reset_point;

  auto success = group->find_reset_point(start_id, reset_point);
  start_pos = reset_point.pos;
  start_id = reset_point.lsn;

  if (!success) {
    arch_mutex_exit();
    DBUG_PRINT("page_archiver",
               ("Can't fetch pages - No matching reset point."));
    return (ER_PAGE_TRACKING_RANGE_NOT_TRACKED);
  }

  /* 3. Fetch tracked pages. */

  DBUG_PRINT("page_archiver",
             ("Trying to get pages between %" PRIu64 " to %" PRIu64 "",
              start_id, stop_id));

  byte header_buf[ARCH_PAGE_BLK_HEADER_LENGTH];

  int err = 0;
  auto cur_pos = start_pos;
  Arch_Page_Pos temp_pos;
  uint num_pages;
  bool new_block = true;
  bool last_block = false;
  lsn_t block_stop_lsn = LSN_MAX;
  uint read_len = 0;
  uint bytes_left = 0;

  arch_oper_mutex_enter();

  auto end_lsn = group->get_end_lsn();

  Arch_Page_Pos last_pos =
      (end_lsn == LSN_MAX) ? m_write_pos : group->get_stop_pos();
  arch_oper_mutex_exit();

  while (true) {
    if (new_block) {
      temp_pos.m_block_num = cur_pos.m_block_num;
      temp_pos.m_offset = 0;

      /* Read the block header for data length and stop lsn info. */
      err = group->read_data(temp_pos, header_buf, ARCH_PAGE_BLK_HEADER_LENGTH);

      if (err != 0) {
        break;
      }

      block_stop_lsn = Arch_Block::get_stop_lsn(header_buf);
      auto data_len = Arch_Block::get_data_len(header_buf);
      bytes_left = data_len + ARCH_PAGE_BLK_HEADER_LENGTH;

      ut_ad(bytes_left <= ARCH_PAGE_BLK_SIZE);
      ut_ad(block_stop_lsn != LSN_MAX);

      bytes_left -= cur_pos.m_offset;

      if (data_len == 0 || cur_pos.m_block_num == last_pos.m_block_num ||
          block_stop_lsn > stop_id) {
        ut_ad(block_stop_lsn >= stop_id);
        stop_id = block_stop_lsn;
        last_block = true;
      }

      DBUG_PRINT("page_archiver",
                 ("%" PRIu64 " -> length : %u, stop lsn : %" PRIu64
                  ", last block : %u",
                  cur_pos.m_block_num, data_len, block_stop_lsn, last_block));
    }

    ut_ad(cur_pos.m_offset <= ARCH_PAGE_BLK_SIZE);

    /* Read how much ever is left to be read in the block. */
    read_len = bytes_left;

    if (last_block && read_len == 0) {
      /* There is nothing to read. */
      break;
    }

    if (read_len > buf_len) {
      read_len = buf_len;
    }

    /* Read the block for list of pages */
    err = group->read_data(cur_pos, buf, read_len);

    if (err != 0) {
      break;
    }

    cur_pos.m_offset += read_len;
    bytes_left -= read_len;
    num_pages = read_len / ARCH_BLK_PAGE_ID_SIZE;

    err = cbk_func(thd, buf, buf_len, num_pages, cbk_ctx);

    if (err != 0) {
      break;
    }

    if (bytes_left == 0) {
      /* We have read all the pages in the block. */

      if (last_block) {
        break;
      } else {
        new_block = true;
        bytes_left = 0;
        read_len = 0;
        cur_pos.set_next();
        continue;
      }
    } else {
      /* We still have some bytes to read from the current block. */
      new_block = false;
    }
  }

  arch_mutex_exit();

  return (0);
}

bool Arch_Page_Sys::get_num_pages(Arch_Page_Pos start_pos,
                                  Arch_Page_Pos stop_pos, uint64_t &num_pages) {
  if (start_pos.m_block_num > stop_pos.m_block_num ||
      ((start_pos.m_block_num == stop_pos.m_block_num) &&
       (start_pos.m_offset >= stop_pos.m_offset))) {
    return (false);
  }

  uint length = 0;

  if (start_pos.m_block_num != stop_pos.m_block_num) {
    length = ARCH_PAGE_BLK_SIZE - start_pos.m_offset;
    length += stop_pos.m_offset - ARCH_PAGE_BLK_HEADER_LENGTH;

    uint64_t num_blocks;
    num_blocks = stop_pos.m_block_num - start_pos.m_block_num - 1;
    length += num_blocks * (ARCH_PAGE_BLK_SIZE - ARCH_PAGE_BLK_HEADER_LENGTH);

  } else {
    length = stop_pos.m_offset - start_pos.m_offset;
  }

  num_pages = length / ARCH_BLK_PAGE_ID_SIZE;

  return (true);
}

int Arch_Page_Sys::get_num_pages(lsn_t &start_id, lsn_t &stop_id,
                                 uint64_t *num_pages) {
  DBUG_PRINT("page_archiver", ("Fetch num pages"));

  arch_mutex_enter();

  /** 1. Get appropriate LSN range. */
  Arch_Group *group = nullptr;

  int error = fetch_group_within_lsn_range(start_id, stop_id, &group);

#ifdef UNIV_DEBUG
  arch_oper_mutex_enter();

  DBUG_PRINT("page_archiver", ("Start id: %" PRIu64 ", stop id: %" PRIu64 "",
                               start_id, stop_id));
  if (is_active()) {
    DBUG_PRINT("page_archiver",
               ("Write_pos : %" PRIu64 ", %u", m_write_pos.m_block_num,
                m_write_pos.m_offset));
  }
  DBUG_PRINT("page_archiver",
             ("Latest stop lsn : %" PRIu64 "", m_latest_stop_lsn));

  arch_oper_mutex_exit();
#endif

  if (error != 0) {
    arch_mutex_exit();
    return (error);
  }

  ut_ad(group != nullptr);

  /** 2. Get block position from where to start. */

  Arch_Point start_point;
  bool success;

  success = group->find_reset_point(start_id, start_point);

  if (!success) {
    DBUG_PRINT("page_archiver",
               ("Can't fetch pages - No matching reset point."));
    arch_mutex_exit();
    return (ER_PAGE_TRACKING_RANGE_NOT_TRACKED);
  }

  DBUG_PRINT(
      "page_archiver",
      ("Start point - lsn : %" PRIu64 " \tpos : %" PRIu64 ", %u",
       start_point.lsn, start_point.pos.m_block_num, start_point.pos.m_offset));

  Arch_Page_Pos start_pos = start_point.pos;
  start_id = start_point.lsn;

  /** 3. Get block position where to stop */

  Arch_Point stop_point;

  success = group->find_stop_point(stop_id, stop_point, m_write_pos);
  ut_ad(success);

  DBUG_PRINT(
      "page_archiver",
      ("Stop point - lsn : %" PRIu64 " \tpos : %" PRIu64 ", %u", stop_point.lsn,
       stop_point.pos.m_block_num, stop_point.pos.m_offset));

  arch_mutex_exit();

  Arch_Page_Pos stop_pos = stop_point.pos;
  stop_id = stop_point.lsn;

  /** 4. Fetch number of pages tracked. */

  ut_ad(start_point.lsn <= stop_point.lsn);
  ut_ad(start_point.pos.m_block_num <= stop_point.pos.m_block_num);

  success = get_num_pages(start_pos, stop_pos, *num_pages);

  if (!success) {
    num_pages = 0;
  }

  DBUG_PRINT("page_archiver",
             ("Number of pages tracked : %" PRIu64 "", *num_pages));

  return (0);
}


/** Check if the gap from last reset is short.
If not many page IDs are added till last reset, we avoid taking a new reset
point
@return true, if the gap is small. */
bool Arch_Page_Sys::is_gap_small() {
  ut_ad(m_last_pos.m_block_num <= m_write_pos.m_block_num);

  if (m_last_pos.m_block_num == m_write_pos.m_block_num) {
    return (true);
  }

  auto next_block_num = m_last_pos.m_block_num + 1;
  auto length = ARCH_PAGE_BLK_SIZE - m_last_pos.m_offset;

  if (next_block_num != m_write_pos.m_block_num) {
    return (false);
  }

  length += m_write_pos.m_offset - ARCH_PAGE_BLK_HEADER_LENGTH;

  /* Pages added since last reset. */
  auto num_pages = length / ARCH_BLK_PAGE_ID_SIZE;

  return (num_pages < ARCH_PAGE_RESET_THRESHOLD);
}

/** Track pages for which IO is already started. */
void Arch_Page_Sys::track_initial_pages() {
  uint index;
  buf_pool_t *buf_pool;

  for (index = 0; index < srv_buf_pool_instances; ++index) {
    buf_pool = buf_pool_from_array(index);

    mutex_enter(&buf_pool->flush_state_mutex);

    /* Page tracking must already be active. */
    ut_ad(buf_pool->track_page_lsn != LSN_MAX);

    buf_flush_list_mutex_enter(buf_pool);

    buf_page_t *bpage;
    uint page_count;
    uint skip_count;

    bpage = UT_LIST_GET_LAST(buf_pool->flush_list);
    page_count = 0;
    skip_count = 0;

    /* Add all pages for which IO is already started. */
    while (bpage != NULL) {
      if (fsp_is_system_temporary(bpage->id.space())) {
        bpage = UT_LIST_GET_PREV(list, bpage);
        continue;
      }

      /* There cannot be any more IO fixed pages. */

      /* Check if we could finish traversing flush list
      earlier. Order of pages in flush list became relaxed,
      but the distortion is limited by the flush_order_lag.

      You can think about this in following way: pages
      start to travel to flush list when they have the
      oldest_modification field assigned. They start in
      proper order, but they can be delayed when traveling
      and they can finish their travel in different order.

      However page is disallowed to finish its travel,
      if there is other page, which started much much
      earlier its travel and still haven't finished.
      The "much much" part is defined by the maximum
      allowed lag - log_buffer_flush_order_lag(). */
      if (bpage->oldest_modification >
          buf_pool->max_lsn_io + log_buffer_flush_order_lag(*log_sys)) {
        /* All pages with oldest_modification
        smaller than bpage->oldest_modification
        minus the flush_order_lag have already
        been traversed. So there is no page which:
                - we haven't traversed
                - and has oldest_modification
                  smaller than buf_pool->max_lsn_io. */
        break;
      }

      if (buf_page_get_io_fix_unlocked(bpage) == BUF_IO_WRITE) {
        /* IO has already started. Must add the page */
        track_page(bpage, LSN_MAX, LSN_MAX, true);
        ++page_count;
      } else {
        ++skip_count;
      }

      bpage = UT_LIST_GET_PREV(list, bpage);
    }

    buf_flush_list_mutex_exit(buf_pool);
    mutex_exit(&buf_pool->flush_state_mutex);
  }
}

/** Enable tracking pages in all buffer pools.
@param[in]	tracking_lsn	track pages from this LSN */
void Arch_Page_Sys::set_tracking_buf_pool(lsn_t tracking_lsn) {
  uint index;
  buf_pool_t *buf_pool;

  for (index = 0; index < srv_buf_pool_instances; ++index) {
    buf_pool = buf_pool_from_array(index);

    mutex_enter(&buf_pool->flush_state_mutex);

    ut_ad(buf_pool->track_page_lsn == LSN_MAX ||
          buf_pool->track_page_lsn <= tracking_lsn);

    buf_pool->track_page_lsn = tracking_lsn;

    mutex_exit(&buf_pool->flush_state_mutex);
  }
}

int Arch_Page_Sys::start_during_recovery(Arch_Group *group,
                                         bool new_empty_file) {
  int err = 0;

  /* Initialise the page archiver with the info parsed from the files. */

  arch_mutex_enter();
  arch_oper_mutex_enter();

  m_current_group = group;

  Arch_Reset_File last_reset_file;
  group->recovery_fetch_info(last_reset_file, m_latest_stop_lsn);

  ut_ad(last_reset_file.m_start_point.size() > 0);

  Arch_Point reset_point = last_reset_file.m_start_point.back();

  m_last_pos = reset_point.pos;
  m_last_lsn = reset_point.lsn;
  m_flush_pos = m_write_pos;
  m_last_reset_file_index = last_reset_file.m_file_index;

  ut_ad(m_last_lsn != LSN_MAX);

  arch_oper_mutex_exit();
  arch_mutex_exit();

  err = m_ctx->init_during_recovery(m_current_group, m_last_lsn);

  if (err != 0) {
    return (err);
  }

  arch_mutex_enter();
  arch_oper_mutex_enter();

  if (new_empty_file) {
    m_flush_pos.set_next();
    m_write_pos.set_next();
    m_reset_pos.set_next();
    m_last_reset_file_index = m_reset_pos.m_block_num;
  }

  /* Reload both the reset block and write block active at the time of
  a crash. */

  auto cur_blk = m_data.get_block(&m_write_pos, ARCH_DATA_BLOCK);
  cur_blk->begin_write(m_write_pos);

  auto reset_block = m_data.get_block(&m_reset_pos, ARCH_RESET_BLOCK);
  reset_block->begin_write(m_write_pos);

  if (!new_empty_file) {
    cur_blk->set_data_len(m_write_pos.m_offset - ARCH_PAGE_BLK_HEADER_LENGTH);
    cur_blk->read(
        m_current_group,
        Arch_Block::get_file_offset(m_write_pos.m_block_num, ARCH_DATA_BLOCK));

    reset_block->set_data_len(m_reset_pos.m_offset -
                              ARCH_PAGE_BLK_HEADER_LENGTH);
    reset_block->read(
        m_current_group,
        Arch_Block::get_file_offset(m_reset_pos.m_block_num, ARCH_RESET_BLOCK));
  }

  ut_d(print());

  arch_oper_mutex_exit();
  arch_mutex_exit();

  return (err);
}

int Arch_Page_Sys::start(Arch_Group **group, lsn_t *start_lsn,
                         Arch_Page_Pos *start_pos, bool is_durable,
                         bool restart, bool recovery) {
  bool start_archiver = true;
  bool attach_to_current = false;
  bool acquired_oper_mutex = false;

  lsn_t log_sys_lsn = LSN_MAX;

  /* Check if archiver task needs to be started. */
  arch_mutex_enter();
  start_archiver = is_init();

  /* Wait for idle state, if preparing to idle. */
  if (!wait_idle()) {
    int err = 0;

    if (srv_shutdown_state != SRV_SHUTDOWN_NONE) {
      err = ER_QUERY_INTERRUPTED;
      my_error(err, MYF(0));
    } else {
      err = ER_INTERNAL_ERROR;
      my_error(err, MYF(0), "Page Archiver wait too long");
    }

    return (err);
  }

  switch (m_state) {
    case ARCH_STATE_ABORT:
      arch_mutex_exit();
      my_error(ER_QUERY_INTERRUPTED, MYF(0));
      return (ER_QUERY_INTERRUPTED);

    case ARCH_STATE_INIT:
    case ARCH_STATE_IDLE:
      /* Fall through */

    case ARCH_STATE_ACTIVE:

      if (m_current_group != nullptr) {
        /* If gap is small, just attach to current group */
        attach_to_current = (recovery ? false : is_gap_small());

        if (attach_to_current) {
          DBUG_PRINT("page_archiver",
                     ("Gap is small - last pos : %" PRIu64
                      " %u, write_pos : %" PRIu64 " %u",
                      m_last_pos.m_block_num, m_last_pos.m_offset,
                      m_write_pos.m_block_num, m_write_pos.m_offset));
        }
      }

      if (!attach_to_current) {
        log_buffer_x_lock_enter(*log_sys);

        if (!recovery) {
          MONITOR_INC(MONITOR_PAGE_TRACK_RESETS);
        }

        log_sys_lsn = (recovery ? m_last_lsn : log_get_lsn(*log_sys));

        /* Enable/Reset buffer pool page tracking. */
        set_tracking_buf_pool(log_sys_lsn);

        /* Take operation mutex before releasing log_sys to
        ensure that all pages modified after log_sys_lsn are
        tracked. */
        arch_oper_mutex_enter();
        acquired_oper_mutex = true;

        log_buffer_x_lock_exit(*log_sys);
      } else {
        arch_oper_mutex_enter();
        acquired_oper_mutex = true;
      }
      break;

    case ARCH_STATE_PREPARE_IDLE:
    default:
      ut_ad(false);
  }

  if (is_init() && !m_data.init()) {
    ut_ad(!attach_to_current);
    acquired_oper_mutex = false;
    arch_oper_mutex_exit();
    arch_mutex_exit();

    my_error(ER_OUTOFMEMORY, MYF(0), ARCH_PAGE_BLK_SIZE);
    return (ER_OUTOFMEMORY);
  }

  /* Start archiver background task. */
  if (start_archiver) {
    ut_ad(!attach_to_current);

    auto err = start_page_archiver_background();

    if (err != 0) {
      acquired_oper_mutex = false;
      arch_oper_mutex_exit();
      arch_mutex_exit();

      ib::error(ER_IB_MSG_26) << "Could not start "
                              << "Archiver background task";
      return (err);
    }
  }

  /* Create a new archive group. */
  if (m_current_group == nullptr) {
    ut_ad(!attach_to_current);

    m_last_pos.init();
    m_flush_pos.init();
    m_write_pos.init();
    m_reset_pos.init();
    m_request_flush_pos.init();
    m_request_blk_num_with_lsn = std::numeric_limits<uint64_t>::max();
    m_flush_blk_num_with_lsn = std::numeric_limits<uint64_t>::max();

    m_last_lsn = log_sys_lsn;
    m_last_reset_file_index = 0;

    m_current_group =
        UT_NEW(Arch_Group(log_sys_lsn, ARCH_PAGE_FILE_HDR_SIZE, &m_mutex),
               mem_key_archive);

    if (m_current_group == nullptr) {
      acquired_oper_mutex = false;
      arch_oper_mutex_exit();
      arch_mutex_exit();

      my_error(ER_OUTOFMEMORY, MYF(0), sizeof(Arch_Group));
      return (ER_OUTOFMEMORY);
    }

    /* Initialize archiver file context. */
    auto db_err = m_current_group->init_file_ctx(
        ARCH_DIR, ARCH_PAGE_DIR, ARCH_PAGE_FILE, 0,
        static_cast<uint64_t>(ARCH_PAGE_BLK_SIZE) * ARCH_PAGE_FILE_CAPACITY);

    if (db_err != DB_SUCCESS) {
      arch_oper_mutex_exit();
      arch_mutex_exit();

      my_error(ER_OUTOFMEMORY, MYF(0), sizeof(Arch_File_Ctx));
      return (ER_OUTOFMEMORY);
    }

    m_group_list.push_back(m_current_group);

    Arch_Block *reset_block = m_data.get_block(&m_reset_pos, ARCH_RESET_BLOCK);
    reset_block->begin_write(m_write_pos);

    DBUG_PRINT("page_archiver", ("Creating a new archived group."));

  } else if (!attach_to_current && !recovery) {
    /* It's a reset. */
    m_last_lsn = log_sys_lsn;
    m_last_pos = m_write_pos;

    DBUG_PRINT("page_archiver", ("It's a reset."));
  }

  m_state = ARCH_STATE_ACTIVE;
  *start_lsn = m_last_lsn;

  if (!recovery) {
    if (!attach_to_current) {
      save_reset_point(is_durable);

    } else if (is_durable && !m_current_group->is_durable()) {
      /* In case this is the first durable archiving of the group and if the
      gap is small for a reset wait for the reset info to be flushed */

      bool success = wait_for_reset_info_flush(m_last_pos.m_block_num);

      if (!success) {
        ib::warn() << "Unable to flush. Page archiving data"
                   << " may be corrupt in case of a crash";
      }
    }
  }

  acquired_oper_mutex = false;
  arch_oper_mutex_exit();

  ut_ad(m_last_lsn != LSN_MAX);
  ut_ad(m_current_group != nullptr);

  if (!restart) {
    /* Add pages to tracking for which IO has already started. */
    track_initial_pages();

    *group = m_current_group;

    *start_pos = m_last_pos;

    arch_oper_mutex_enter();
    acquired_oper_mutex = true;

    /* Attach to the group. */
    m_current_group->attach(is_durable);

    if (is_durable && !recovery) {
      m_current_group->mark_active();
      m_current_group->mark_durable();
    }
  } else if (recovery) {
    arch_oper_mutex_enter();
    acquired_oper_mutex = true;

    /* Attach to the group. */
    m_current_group->attach(is_durable);
  }

  ut_ad(*group == m_current_group);

  if (acquired_oper_mutex) {
    arch_oper_mutex_exit();
  }

  arch_mutex_exit();

  if (!recovery) {
    /* Make sure all written pages are synced to disk. */
    log_request_checkpoint(*log_sys, false);
  }

  return (0);
}

int Arch_Page_Sys::stop(Arch_Group *group, lsn_t *stop_lsn,
                        Arch_Page_Pos *stop_pos, bool is_durable) {
  Arch_Block *cur_blk;

  arch_mutex_enter();

  ut_ad(group == m_current_group);
  ut_ad(m_state == ARCH_STATE_ACTIVE);

  arch_oper_mutex_enter();

  *stop_lsn = m_latest_stop_lsn;
  cur_blk = m_data.get_block(&m_write_pos, ARCH_DATA_BLOCK);
  update_stop_info(cur_blk);

  auto count = group->detach(*stop_lsn, &m_write_pos);
  arch_oper_mutex_exit();

  int err = 0;

  /* If no other active client, let the system get into idle state. */
  if (count == 0 && m_state != ARCH_STATE_ABORT) {
    set_tracking_buf_pool(LSN_MAX);

    arch_oper_mutex_enter();

    m_state = ARCH_STATE_PREPARE_IDLE;
    *stop_pos = m_write_pos;

    cur_blk->end_write();
    m_request_flush_pos = m_write_pos;
    m_write_pos.set_next();

    os_event_set(page_archiver_thread_event);

    if (m_current_group->is_durable()) {
      /* Wait for flush archiver to flush the blocks. */
      auto cbk = [&] {
        if (m_flush_pos.m_block_num > m_request_flush_pos.m_block_num) {
          return (false);
        }
        return (true);
      };

      bool success = wait_flush_archiver(cbk);
      if (!success) {
        ib::warn() << "Unable to flush. Page archiving data"
                   << " may be corrupt in case of a crash";
      }

      ut_ad(group->validate_info_in_files());
    }

  } else {
    if (m_state != ARCH_STATE_ABORT && is_durable &&
        !m_current_group->is_durable_client_active()) {
      /* In case the non-durable clients are still active but there are no
      active durable clients we need to mark the group inactive for recovery
      to know that no durable clients were active. */
      err = m_current_group->mark_inactive();
    }

    arch_oper_mutex_enter();

    *stop_pos = m_write_pos;
  }

  if (m_state == ARCH_STATE_ABORT) {
    my_error(ER_QUERY_INTERRUPTED, MYF(0));
    err = ER_QUERY_INTERRUPTED;
  }

  arch_oper_mutex_exit();
  arch_mutex_exit();

  return (err);
}

void Arch_Page_Sys::release(Arch_Group *group, bool is_durable,
                            Arch_Page_Pos start_pos) {
  arch_mutex_enter();
  arch_oper_mutex_enter();

  group->release(is_durable);

  arch_oper_mutex_exit();

  if (group->is_active()) {
    arch_mutex_exit();
    return;
  }

  ut_ad(group != m_current_group);

  if (!group->is_referenced()) {
    m_group_list.remove(group);
    UT_DELETE(group);
  }

  arch_mutex_exit();
}

dberr_t Arch_Page_Sys::flush_inactive_blocks(Arch_Page_Pos &cur_pos,
                                             Arch_Page_Pos end_pos) {
  dberr_t err = DB_SUCCESS;
  Arch_Block *cur_blk;

  /* Write all blocks that are ready for flushing. */
  while (cur_pos.m_block_num < end_pos.m_block_num) {
    cur_blk = m_data.get_block(&cur_pos, ARCH_DATA_BLOCK);

    err = cur_blk->flush(m_current_group, ARCH_FLUSH_NORMAL);

    if (err != DB_SUCCESS) {
      break;
    }

    MONITOR_INC(MONITOR_PAGE_TRACK_FULL_BLOCK_WRITES);

    arch_oper_mutex_enter();

    m_flush_blk_num_with_lsn = cur_pos.m_block_num;
    cur_pos.set_next();
    cur_blk->set_flushed();
    m_flush_pos.set_next();

    arch_oper_mutex_exit();
  }

  return (err);
}

dberr_t Arch_Page_Sys::flush_active_block(Arch_Page_Pos cur_pos,
                                          bool partial_reset_block_flush) {
  Arch_Block *cur_blk;
  cur_blk = m_data.get_block(&cur_pos, ARCH_DATA_BLOCK);

  arch_oper_mutex_enter();

  if (!cur_blk->is_active()) {
    arch_oper_mutex_exit();
    return (DB_SUCCESS);
  }

  /* Copy block data so that we can release the arch_oper_mutex soon. */
  Arch_Block *flush_blk = m_data.get_partial_flush_block();
  flush_blk->copy_data(cur_blk);

  arch_oper_mutex_exit();

  dberr_t err = flush_blk->flush(m_current_group, ARCH_FLUSH_PARTIAL);

  if (err != DB_SUCCESS) {
    return (err);
  }

  MONITOR_INC(MONITOR_PAGE_TRACK_PARTIAL_BLOCK_WRITES);

  if (partial_reset_block_flush) {
    arch_oper_mutex_enter();

    Arch_Block *reset_block = m_data.get_block(&m_reset_pos, ARCH_RESET_BLOCK);

    arch_oper_mutex_exit();

    err = reset_block->flush(m_current_group, ARCH_FLUSH_NORMAL);

    if (err != DB_SUCCESS) {
      return (err);
    }
  }

  arch_oper_mutex_enter();

  m_flush_pos.m_offset =
      flush_blk->get_data_len() + ARCH_PAGE_BLK_HEADER_LENGTH;

  arch_oper_mutex_exit();

  return (err);
}

dberr_t Arch_Page_Sys::flush_blocks(bool *wait) {
  arch_oper_mutex_enter();

  auto request_flush_pos = m_request_flush_pos;
  auto cur_pos = m_flush_pos;
  auto end_pos = m_write_pos;
  auto request_blk_num_with_lsn = m_request_blk_num_with_lsn;
  auto flush_blk_num_with_lsn = m_flush_blk_num_with_lsn;

  arch_oper_mutex_exit();

  uint64_t ARCH_UNKNOWN_BLOCK = std::numeric_limits<uint64_t>::max();

  ut_ad(cur_pos.m_block_num <= end_pos.m_block_num);

  /* Caller needs to wait/sleep, if nothing to flush. */
  *wait = (cur_pos.m_block_num == end_pos.m_block_num);

  dberr_t err;

  err = flush_inactive_blocks(cur_pos, end_pos);

  if (err != DB_SUCCESS) {
    return (err);
  }

  if (cur_pos.m_block_num == end_pos.m_block_num) {
    /* Partial Flush */

    bool data_block_flush =
        request_flush_pos.m_block_num == cur_pos.m_block_num &&
        request_flush_pos.m_offset > cur_pos.m_offset;
    bool reset_block_flush =
        request_blk_num_with_lsn != ARCH_UNKNOWN_BLOCK &&
        (flush_blk_num_with_lsn == ARCH_UNKNOWN_BLOCK ||
         request_blk_num_with_lsn > flush_blk_num_with_lsn);

    /* We do partial flush only if we're explicitly requested to flush. */
    if (data_block_flush || reset_block_flush) {
      err = flush_active_block(cur_pos, reset_block_flush);

      if (err != DB_SUCCESS) {
        return (err);
      }
    }

    arch_oper_mutex_enter();

    if (request_blk_num_with_lsn != ARCH_UNKNOWN_BLOCK &&
        (flush_blk_num_with_lsn == ARCH_UNKNOWN_BLOCK ||
         request_blk_num_with_lsn > flush_blk_num_with_lsn)) {
      m_flush_blk_num_with_lsn = request_blk_num_with_lsn;
    }

    arch_oper_mutex_exit();
  }

  return (err);
}

bool Arch_Page_Sys::archive(bool *wait) {
  dberr_t db_err;

  auto is_abort = (srv_shutdown_state == SRV_SHUTDOWN_LAST_PHASE ||
                   srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS ||
                   m_state == ARCH_STATE_ABORT);

  arch_oper_mutex_enter();

  /* Check if archiving state is inactive. */
  if (m_state == ARCH_STATE_IDLE || m_state == ARCH_STATE_INIT) {
    *wait = true;

    if (is_abort) {
      m_state = ARCH_STATE_ABORT;
      arch_oper_mutex_exit();

      return (true);
    }

    arch_oper_mutex_exit();

    return (false);
  }

  /* ARCH_STATE_ABORT is set for flush timeout which is asserted in debug. */
  ut_ad(m_state == ARCH_STATE_ACTIVE || m_state == ARCH_STATE_PREPARE_IDLE);

  auto set_idle = (m_state == ARCH_STATE_PREPARE_IDLE);
  arch_oper_mutex_exit();

  db_err = flush_blocks(wait);

  if (db_err != DB_SUCCESS) {
    is_abort = true;
  }

  /* Move to idle state or abort, if needed. */
  if (set_idle || is_abort) {
    arch_mutex_enter();
    arch_oper_mutex_enter();

    m_current_group->disable(LSN_MAX);
    m_current_group->close_file_ctxs();

    int err = 0;

    if (!is_abort && m_current_group->is_durable()) {
      err = m_current_group->mark_inactive();

      Arch_Group::init_dblwr_file_ctx(
          ARCH_DBLWR_DIR, ARCH_DBLWR_FILE, ARCH_DBLWR_NUM_FILES,
          static_cast<uint64_t>(ARCH_PAGE_BLK_SIZE) * ARCH_DBLWR_FILE_CAPACITY);

      ut_ad(m_current_group->validate_info_in_files());
    }

    if (err != 0) {
      is_abort = true;
    }

    /* Cleanup group, if no reference. */
    if (!m_current_group->is_referenced()) {
      m_group_list.remove(m_current_group);
      UT_DELETE(m_current_group);
    }

    m_current_group = nullptr;
    m_state = is_abort ? ARCH_STATE_ABORT : ARCH_STATE_IDLE;

    arch_oper_mutex_exit();
    arch_mutex_exit();
  }

  return (is_abort);
}

void Arch_Page_Sys::save_reset_point(bool is_durable) {
  /* 1. Add the reset info to the reset block */

  uint current_file_index = Arch_Block::get_file_index(m_last_pos.m_block_num);

  auto reset_block = m_data.get_block(&m_reset_pos, ARCH_RESET_BLOCK);

  /* If the reset info should belong to a new file then re-intialize the
  block as the block from now on will contain reset information belonging
  to the new file */
  if (m_last_reset_file_index != current_file_index) {
    ut_ad(current_file_index > m_last_reset_file_index);
    reset_block->begin_write(m_last_pos);
  }

  m_last_reset_file_index = current_file_index;

  reset_block->add_reset(m_last_lsn, m_last_pos);

  m_current_group->save_reset_point_in_mem(m_last_lsn, m_last_pos);

  auto cur_block = m_data.get_block(&m_last_pos, ARCH_DATA_BLOCK);

  if (cur_block->get_state() == ARCH_BLOCK_INIT ||
      cur_block->get_state() == ARCH_BLOCK_FLUSHED) {
    cur_block->begin_write(m_last_pos);
  }

  m_latest_stop_lsn = log_get_checkpoint_lsn(*log_sys);
  update_stop_info(cur_block);

  /* 2. Add the reset lsn to the current write_pos block header and request the
  flush archiver to flush the data block and reset block */

  cur_block->update_block_header(LSN_MAX, m_last_lsn);

  ut_d(auto ARCH_UNKNOWN_BLOCK = std::numeric_limits<uint64_t>::max());

  /* Reset LSN for a block can be updated only once. */
  ut_ad(m_flush_blk_num_with_lsn == ARCH_UNKNOWN_BLOCK ||
        m_flush_blk_num_with_lsn < cur_block->get_number());
  ut_ad(m_request_blk_num_with_lsn == ARCH_UNKNOWN_BLOCK ||
        m_request_blk_num_with_lsn < cur_block->get_number());

  uint64_t request_blk_num_with_lsn = cur_block->get_number();

  m_request_blk_num_with_lsn = request_blk_num_with_lsn;

  /* 3. In case of durable archiving wait till the reset block and the data
  block with reset LSN is flushed so that we do not lose reset information in
  case of a crash. This is not required for non-durable archiving. */
  if (is_durable) {
    bool success = wait_for_reset_info_flush(request_blk_num_with_lsn);

    if (!success) {
      ib::warn() << "Unable to flush. Page archiving data"
                 << " may be corrupt in case of a crash";
    }

    ut_ad(m_current_group->get_file_count());
    ut_ad(current_file_index < m_current_group->get_file_count());
  }

  DBUG_PRINT("page_archiver",
             ("Saved reset point at %u - %" PRIu64 ", %" PRIu64 ", %u\n",
              m_last_reset_file_index, m_last_lsn, m_last_pos.m_block_num,
              m_last_pos.m_offset));
}

bool Arch_Page_Sys::wait_for_reset_info_flush(uint64_t request_blk) {
  auto ARCH_UNKNOWN_BLOCK = std::numeric_limits<uint64_t>::max();

  auto cbk = [&] {
    if (m_flush_blk_num_with_lsn == ARCH_UNKNOWN_BLOCK ||
        request_blk > m_flush_blk_num_with_lsn) {
      return (true);
    }

    return (false);
  };

  bool success = wait_flush_archiver(cbk);

  return (success);
}

int Arch_Page_Sys::fetch_group_within_lsn_range(lsn_t &start_id, lsn_t &stop_id,
                                                Arch_Group **group) {
  ut_ad(mutex_own(&m_mutex));

  if (start_id != 0 && stop_id != 0 && start_id >= stop_id) {
    return (ER_PAGE_TRACKING_RANGE_NOT_TRACKED);
  }

  arch_oper_mutex_enter();
  auto latest_stop_lsn = m_latest_stop_lsn;
  arch_oper_mutex_exit();

  ut_ad(latest_stop_lsn != LSN_MAX);

  if (start_id == 0 || stop_id == 0) {
    if (m_current_group == nullptr || !m_current_group->is_active()) {
      return (ER_PAGE_TRACKING_RANGE_NOT_TRACKED);
    }

    *group = m_current_group;

    ut_ad(m_last_lsn != LSN_MAX);

    start_id = (start_id == 0) ? m_last_lsn : start_id;
    stop_id = (stop_id == 0) ? latest_stop_lsn : stop_id;
  }

  if (start_id >= stop_id || start_id == LSN_MAX || stop_id == LSN_MAX) {
    return (ER_PAGE_TRACKING_RANGE_NOT_TRACKED);
  }

  if (*group == nullptr) {
    for (auto it : m_group_list) {
      *group = it;

      if (start_id < (*group)->get_begin_lsn() ||
          (!(*group)->is_active() && stop_id > (*group)->get_end_lsn()) ||
          ((*group)->is_active() && stop_id > latest_stop_lsn)) {
        *group = nullptr;
        continue;
      }

      break;
    }
  }

  if (*group == nullptr) {
    return (ER_PAGE_TRACKING_RANGE_NOT_TRACKED);
  }

  return (0);
}

uint Arch_Page_Sys::purge(lsn_t *purge_lsn) {
  lsn_t purged_lsn = LSN_MAX;
  uint err = 0;

  if (*purge_lsn == 0) {
    *purge_lsn = log_get_checkpoint_lsn(*log_sys);
  }

  DBUG_PRINT("page_archiver", ("Purging of files - %" PRIu64 "", *purge_lsn));

  arch_mutex_enter();

  for (auto it = m_group_list.begin(); it != m_group_list.end();) {
    lsn_t group_purged_lsn = LSN_MAX;
    auto group = *it;

    DBUG_PRINT("page_archiver",
               ("End lsn - %" PRIu64 "", group->get_end_lsn()));

    err = group->purge(*purge_lsn, group_purged_lsn);

    if (group_purged_lsn == LSN_MAX) {
      break;
    }

    DBUG_PRINT("page_archiver",
               ("Group purged lsn - %" PRIu64 "", group_purged_lsn));

    if (purged_lsn == LSN_MAX || group_purged_lsn > purged_lsn) {
      purged_lsn = group_purged_lsn;
    }

    if (!group->is_active() && group->get_end_lsn() <= group_purged_lsn) {
      it = m_group_list.erase(it);
      UT_DELETE(group);

      DBUG_PRINT("page_archiver", ("Purged entire group."));

      continue;
    }

    ++it;
  }

  DBUG_PRINT("page_archiver",
             ("Purged archived file until : %" PRIu64 "", purged_lsn));

  *purge_lsn = purged_lsn;

  if (purged_lsn == LSN_MAX) {
    arch_mutex_exit();
    return (err);
  }

  m_latest_purged_lsn = purged_lsn;

  arch_mutex_exit();

  return (err);
}

void Arch_Page_Sys::update_stop_info(Arch_Block *cur_blk) {
  ut_ad(mutex_own(&m_oper_mutex));

  if (cur_blk != nullptr) {
    cur_blk->update_block_header(m_latest_stop_lsn, LSN_MAX);
  }

  if (m_current_group != nullptr) {
    m_current_group->update_stop_point(m_write_pos, m_latest_stop_lsn);
  }
}

#ifdef UNIV_DEBUG
void Arch_Page_Sys::print() {
  DBUG_PRINT("page_archiver", ("State : %u", m_state));
  DBUG_PRINT("page_archiver", ("Last pos : %" PRIu64 ", %u",
                               m_last_pos.m_block_num, m_last_pos.m_offset));
  DBUG_PRINT("page_archiver", ("Last lsn : %" PRIu64 "", m_last_lsn));
  DBUG_PRINT("page_archiver",
             ("Latest stop lsn : %" PRIu64 "", m_latest_stop_lsn));
  DBUG_PRINT("page_archiver", ("Flush pos : %" PRIu64 ", %u",
                               m_flush_pos.m_block_num, m_flush_pos.m_offset));
  DBUG_PRINT("page_archiver", ("Write pos : %" PRIu64 ", %u",
                               m_write_pos.m_block_num, m_write_pos.m_offset));
  DBUG_PRINT("page_archiver", ("Reset pos : %" PRIu64 ", %u",
                               m_reset_pos.m_block_num, m_reset_pos.m_offset));
  DBUG_PRINT("page_archiver",
             ("Last reset file index : %u", m_last_reset_file_index));

  Arch_Block *reset_block = m_data.get_block(&m_reset_pos, ARCH_RESET_BLOCK);
  Arch_Block *data_block = m_data.get_block(&m_write_pos, ARCH_DATA_BLOCK);

  DBUG_PRINT("page_archiver", ("Latest reset block data length: %u",
                               reset_block->get_data_len()));
  DBUG_PRINT("page_archiver",
             ("Latest data block data length: %u", data_block->get_data_len()));
}
#endif
