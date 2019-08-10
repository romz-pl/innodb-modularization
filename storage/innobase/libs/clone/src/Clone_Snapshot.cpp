#include <innodb/clone/Clone_Snapshot.h>

#include <innodb/log_types/flags.h>
#include <innodb/io/Fil_path.h>
#include <innodb/log_types/flags.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/buffer/Page_fetch.h>
#include <innodb/clone/pfs.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/srv_is_direct_io.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/logger/info.h>
#include <innodb/memory/mem_heap_create.h>
#include <innodb/memory/mem_heap_free.h>
#include <innodb/mtr/mtr_start.h>
#include <innodb/page/page_zip_set_size.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_space_get_page_size.h>
#include <innodb/tablespace/fsp_header_get_field.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/macros.h>
#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/tablespace/Fil_iterator.h>
#include <innodb/clone/add_redo_file_callback.h>
#include <innodb/clone/add_page_callback.h>
#include <innodb/io/os_file_size_t.h>
#include <innodb/clone/is_ddl_temp_table.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/io/os_file_get_size.h>

/** Number of clones that can attach to a snapshot. */
const uint MAX_CLONES_PER_SNAPSHOT = 1;

/** Snapshot heap initial size */
const uint SNAPSHOT_MEM_INITIAL_SIZE = 16 * 1024;

Clone_Snapshot::Clone_Snapshot(Clone_Handle_Type hdl_type,
                               Ha_clone_type clone_type, uint arr_idx,
                               ib_uint64_t snap_id)
    : m_snapshot_handle_type(hdl_type),
      m_snapshot_type(clone_type),
      m_snapshot_id(snap_id),
      m_snapshot_arr_idx(arr_idx),
      m_allow_new_clone(true),
      m_num_clones(),
      m_num_clones_current(),
      m_num_clones_next(),
      m_snapshot_state(CLONE_SNAPSHOT_INIT),
      m_snapshot_next_state(CLONE_SNAPSHOT_NONE),
      m_num_current_chunks(),
      m_max_file_name_len(),
      m_num_data_files(),
      m_num_data_chunks(),
      m_page_ctx(false),
      m_num_pages(),
      m_num_duplicate_pages(),
      m_redo_ctx(),
      m_redo_start_offset(),
      m_redo_header(),
      m_redo_header_size(),
      m_redo_trailer(),
      m_redo_trailer_size(),
      m_redo_trailer_offset(),
      m_redo_file_size(),
      m_num_redo_files(),
      m_num_redo_chunks(),
      m_enable_pfs(false) {
  mutex_create(LATCH_ID_CLONE_SNAPSHOT, &m_snapshot_mutex);

  m_snapshot_heap = mem_heap_create(SNAPSHOT_MEM_INITIAL_SIZE);

  m_chunk_size_pow2 = SNAPSHOT_DEF_CHUNK_SIZE_POW2;
  m_block_size_pow2 = SNAPSHOT_DEF_BLOCK_SIZE_POW2;
}

Clone_Snapshot::~Clone_Snapshot() {
  m_redo_ctx.release();

  if (m_page_ctx.is_active()) {
    m_page_ctx.stop(nullptr);
  }
  m_page_ctx.release();

  mem_heap_free(m_snapshot_heap);

  mutex_free(&m_snapshot_mutex);
}

void Clone_Snapshot::get_state_info(bool do_estimate,
                                    Clone_Desc_State *state_desc) {
  state_desc->m_state = m_snapshot_state;
  state_desc->m_num_chunks = m_num_current_chunks;

  state_desc->m_is_start = true;
  state_desc->m_is_ack = false;
  state_desc->m_estimate = 0;

  if (do_estimate) {
    state_desc->m_estimate = m_monitor.get_estimate();
  }

  switch (m_snapshot_state) {
    case CLONE_SNAPSHOT_FILE_COPY:
      state_desc->m_num_files = m_num_data_files;
      break;

    case CLONE_SNAPSHOT_PAGE_COPY:
      state_desc->m_num_files = m_num_pages;
      break;

    case CLONE_SNAPSHOT_REDO_COPY:
      state_desc->m_num_files = m_num_redo_files;

      /* Minimum of two redo files need to be created. */
      if (state_desc->m_num_files < 2) {
        state_desc->m_num_files = 2;
      }
      break;

    case CLONE_SNAPSHOT_DONE:
      /* fall thorugh */

    case CLONE_SNAPSHOT_INIT:
      state_desc->m_num_files = 0;
      break;

    default:
      ut_ad(false);
  }
}

void Clone_Snapshot::set_state_info(Clone_Desc_State *state_desc) {
  ut_ad(mutex_own(&m_snapshot_mutex));
  ut_ad(state_desc->m_state == m_snapshot_state);

  m_num_current_chunks = state_desc->m_num_chunks;

  if (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY) {
    m_num_data_files = state_desc->m_num_files;
    m_num_data_chunks = state_desc->m_num_chunks;
    m_data_file_vector.resize(m_num_data_files, nullptr);

    m_monitor.init_state(srv_stage_clone_file_copy.m_key, m_enable_pfs);
    m_monitor.add_estimate(state_desc->m_estimate);
    m_monitor.change_phase();

  } else if (m_snapshot_state == CLONE_SNAPSHOT_PAGE_COPY) {
    m_num_pages = state_desc->m_num_files;

    m_monitor.init_state(srv_stage_clone_page_copy.m_key, m_enable_pfs);
    m_monitor.add_estimate(state_desc->m_estimate);
    m_monitor.change_phase();

  } else if (m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY) {
    m_num_redo_files = state_desc->m_num_files;
    m_num_redo_chunks = state_desc->m_num_chunks;
    m_redo_file_vector.resize(m_num_redo_files, nullptr);

    m_monitor.init_state(srv_stage_clone_redo_copy.m_key, m_enable_pfs);
    m_monitor.add_estimate(state_desc->m_estimate);
    m_monitor.change_phase();

  } else if (m_snapshot_state == CLONE_SNAPSHOT_DONE) {
    ut_ad(m_num_current_chunks == 0);
    m_monitor.init_state(PSI_NOT_INSTRUMENTED, m_enable_pfs);

  } else {
    ut_ad(false);
  }
}

Snapshot_State Clone_Snapshot::get_next_state() {
  Snapshot_State next_state;

  ut_ad(m_snapshot_state != CLONE_SNAPSHOT_NONE);

  if (m_snapshot_state == CLONE_SNAPSHOT_INIT) {
    next_state = CLONE_SNAPSHOT_FILE_COPY;

  } else if (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY) {
    if (m_snapshot_type == HA_CLONE_HYBRID ||
        m_snapshot_type == HA_CLONE_PAGE) {
      next_state = CLONE_SNAPSHOT_PAGE_COPY;

    } else if (m_snapshot_type == HA_CLONE_REDO) {
      next_state = CLONE_SNAPSHOT_REDO_COPY;

    } else {
      ut_ad(m_snapshot_type == HA_CLONE_BLOCKING);
      next_state = CLONE_SNAPSHOT_DONE;
    }

  } else if (m_snapshot_state == CLONE_SNAPSHOT_PAGE_COPY) {
    next_state = CLONE_SNAPSHOT_REDO_COPY;

  } else {
    ut_ad(m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY);
    next_state = CLONE_SNAPSHOT_DONE;
  }

  return (next_state);
}

bool Clone_Snapshot::attach(Clone_Handle_Type hdl_type, bool pfs_monitor) {
  bool ret = false;
  mutex_enter(&m_snapshot_mutex);

  if (m_num_clones == 0) {
    m_enable_pfs = pfs_monitor;
  }

  if (m_allow_new_clone && hdl_type == m_snapshot_handle_type &&
      m_num_clones < MAX_CLONES_PER_SNAPSHOT) {
    ++m_num_clones;

    if (in_transit_state()) {
      ++m_num_clones_current;
    }

    ret = true;
  }

  mutex_exit(&m_snapshot_mutex);
  return (ret);
}




uint Clone_Snapshot::detach() {
  uint num_clones_left;

  mutex_enter(&m_snapshot_mutex);

  ut_ad(m_num_clones > 0);

  if (in_transit_state()) {
    --m_num_clones_current;
  }

  num_clones_left = --m_num_clones;

  mutex_exit(&m_snapshot_mutex);

  return (num_clones_left);
}

int Clone_Snapshot::change_state(Clone_Desc_State *state_desc,
                                 Snapshot_State new_state, byte *temp_buffer,
                                 uint temp_buffer_len, uint &pending_clones) {
  ut_ad(m_snapshot_state != CLONE_SNAPSHOT_NONE);

  mutex_enter(&m_snapshot_mutex);

  if (m_snapshot_state != CLONE_SNAPSHOT_INIT) {
    m_allow_new_clone = false;
  }

  /* Initialize transition if not started yet by other clones. */
  if (!in_transit_state()) {
    m_num_clones_current = m_num_clones;

    m_snapshot_next_state = new_state;
    m_num_clones_next = 0;
  }

  /* Move clone over to next state */
  --m_num_clones_current;
  ++m_num_clones_next;

  pending_clones = m_num_clones_current;

  /* Need to wait for other clones to move over. */
  if (pending_clones > 0) {
    mutex_exit(&m_snapshot_mutex);
    return (0);
  }

  /* Last clone requesting the state change. All other clones have
  already moved over to next state and waiting for the transition
  to complete. Now it is safe to do the snapshot state transition. */
  m_snapshot_state = m_snapshot_next_state;

  m_snapshot_next_state = CLONE_SNAPSHOT_NONE;

  m_num_clones_current = 0;
  m_num_clones_next = 0;

  /* Initialize the new state. */
  auto err = init_state(state_desc, temp_buffer, temp_buffer_len);

  mutex_exit(&m_snapshot_mutex);

  return (err);
}

uint Clone_Snapshot::check_state(Snapshot_State new_state, bool exit_on_wait) {
  uint pending_clones;

  mutex_enter(&m_snapshot_mutex);

  pending_clones = 0;
  if (in_transit_state() && new_state == m_snapshot_next_state) {
    pending_clones = m_num_clones_current;
  }

  if (pending_clones != 0 && exit_on_wait) {
    ++m_num_clones_current;
    --m_num_clones_next;
  }

  mutex_exit(&m_snapshot_mutex);

  return (pending_clones);
}

Clone_File_Meta *Clone_Snapshot::get_file_by_index(uint index) {
  Clone_File_Meta *file_meta;

  if (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY ||
      m_snapshot_state == CLONE_SNAPSHOT_PAGE_COPY) {
    ut_ad(index < m_num_data_files);
    file_meta = m_data_file_vector[index];

  } else if (m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY) {
    ut_ad(index < m_num_redo_files);
    file_meta = m_redo_file_vector[index];

  } else {
    ut_ad(false);
    file_meta = nullptr;
  }

  return (file_meta);
}

int Clone_Snapshot::iterate_files(File_Cbk_Func &&func) {
  if (m_snapshot_state != CLONE_SNAPSHOT_FILE_COPY &&
      m_snapshot_state != CLONE_SNAPSHOT_REDO_COPY) {
    return (0);
  }

  auto &file_vector = (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY)
                          ? m_data_file_vector
                          : m_redo_file_vector;

  for (auto file_meta : file_vector) {
    auto err = func(file_meta);
    if (err != 0) {
      return (err);
    }
  }

  return (0);
}

int Clone_Snapshot::get_next_block(uint chunk_num, uint &block_num,
                                   Clone_File_Meta *file_meta,
                                   ib_uint64_t &data_offset, byte *&data_buf,
                                   uint &data_size) {
  uint64_t start_offset = 0;
  uint start_index;
  Clone_File_Meta *current_file;

  /* File index for last chunk. This index value is always increasing
  for a task. We skip all previous index while searching for new file. */
  start_index = file_meta->m_file_index;

  if (m_snapshot_state == CLONE_SNAPSHOT_PAGE_COPY) {
    /* Copy the page from buffer pool. */
    auto err = get_next_page(chunk_num, block_num, file_meta, data_offset,
                             data_buf, data_size);
    return (err);

  } else if (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY) {
    /* Get file for the chunk. */
    current_file =
        get_file(m_data_file_vector, m_num_data_files, chunk_num, start_index);
  } else {
    /* For redo copy header and trailer are returned in buffer. */
    ut_ad(m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY);

    if (chunk_num == (m_num_current_chunks - 1)) {
      /* Last but one chunk is the redo header. */

      if (block_num != 0) {
        block_num = 0;
        return (0);
      }

      ++block_num;

      current_file = m_redo_file_vector.front();
      *file_meta = *current_file;

      data_offset = 0;

      data_buf = m_redo_header;
      ut_ad(data_buf != nullptr);

      data_size = m_redo_header_size;

      return (0);

    } else if (chunk_num == m_num_current_chunks) {
      /* Last chunk is the redo trailer. */

      if (block_num != 0 || m_redo_trailer_size == 0) {
        block_num = 0;
        return (0);
      }

      ++block_num;

      current_file = m_redo_file_vector.back();
      *file_meta = *current_file;

      data_offset = m_redo_trailer_offset;

      data_buf = m_redo_trailer;
      ut_ad(data_buf != nullptr);

      data_size = m_redo_trailer_size;

      return (0);
    }

    /* This is not header or trailer chunk. Need to get redo
    data from archived file. */
    current_file =
        get_file(m_redo_file_vector, m_num_redo_files, chunk_num, start_index);

    if (current_file->m_begin_chunk == 1) {
      /* Set start offset for the first file. */
      start_offset = m_redo_start_offset;
    }

    /* Dummy redo file entry. Need to send metadata. */
    if (current_file->m_file_size == 0) {
      if (block_num != 0) {
        block_num = 0;
        return (0);
      }
      ++block_num;

      *file_meta = *current_file;
      data_buf = nullptr;
      data_size = 0;
      data_offset = 0;

      return (0);
    }
  }

  /* We have identified the file to transfer data at this point.
  Get the data offset for next block to transfer. */
  uint num_blocks;

  data_buf = nullptr;

  uint64_t file_chnuk_num = chunk_num - current_file->m_begin_chunk;

  /* Offset in pages for current chunk. */
  uint64_t chunk_offset = file_chnuk_num << m_chunk_size_pow2;

  /* Find number of blocks in current chunk. */
  if (chunk_num == current_file->m_end_chunk) {
    /* If it is last chunk, we need to adjust the size. */
    ib_uint64_t size_in_pages;
    uint aligned_sz;

    ut_ad(current_file->m_file_size >= start_offset);
    size_in_pages = ut_uint64_align_up(current_file->m_file_size - start_offset,
                                       UNIV_PAGE_SIZE);
    size_in_pages /= UNIV_PAGE_SIZE;

    ut_ad(size_in_pages >= chunk_offset);
    size_in_pages -= chunk_offset;

    aligned_sz = static_cast<uint>(size_in_pages);
    ut_ad(aligned_sz == size_in_pages);

    aligned_sz = ut_calc_align(aligned_sz, block_size());

    num_blocks = aligned_sz >> m_block_size_pow2;
  } else {
    num_blocks = blocks_per_chunk();
  }

  /* Current block is the last one. No more blocks in current chunk. */
  if (block_num == num_blocks) {
    block_num = 0;
    return (0);
  }

  ut_ad(block_num < num_blocks);

  /* Calculate the offset of next block. */
  ib_uint64_t block_offset;

  block_offset = static_cast<ib_uint64_t>(block_num);
  block_offset *= block_size();

  data_offset = chunk_offset + block_offset;
  data_size = block_size();

  ++block_num;

  *file_meta = *current_file;

  /* Convert offset and length in bytes. */
  data_size *= UNIV_PAGE_SIZE;
  data_offset *= UNIV_PAGE_SIZE;
  data_offset += start_offset;

  ut_ad(data_offset < current_file->m_file_size);

  /* Adjust length for last block in last chunk. */
  if (chunk_num == current_file->m_end_chunk && block_num == num_blocks) {
    ut_ad((data_offset + data_size) >= current_file->m_file_size);
    data_size = static_cast<uint>(current_file->m_file_size - data_offset);
  }

#ifdef UNIV_DEBUG
  if (m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY) {
    /* Current file is the last redo file */
    if (current_file == m_redo_file_vector.back() && m_redo_trailer_size != 0) {
      /* Should not exceed/overwrite the trailer */
      ut_ad(data_offset + data_size <= m_redo_trailer_offset);
    }
  }
#endif /* UNIV_DEBUG */

  return (0);
}

void Clone_Snapshot::update_block_size(uint buff_size) {
  mutex_enter(&m_snapshot_mutex);

  /* Transfer data block is used only for direct IO. */
  if (m_snapshot_state != CLONE_SNAPSHOT_INIT || !srv_is_direct_io()) {
    mutex_exit(&m_snapshot_mutex);
    return;
  }

  /* Try to set block size bigger than the transfer buffer. */
  while (buff_size > (block_size() * UNIV_PAGE_SIZE) &&
         m_block_size_pow2 < SNAPSHOT_MAX_BLOCK_SIZE_POW2) {
    ++m_block_size_pow2;
  }

  mutex_exit(&m_snapshot_mutex);
}




int Clone_Snapshot::init_state(Clone_Desc_State *state_desc, byte *temp_buffer,
                               uint temp_buffer_len) {
  int err = 0;
  m_num_current_chunks = 0;

  if (!is_copy()) {
    err = init_apply_state(state_desc);
    return (err);
  }

  switch (m_snapshot_state) {
    case CLONE_SNAPSHOT_NONE:
    case CLONE_SNAPSHOT_INIT:
      ut_ad(false);

      err = ER_INTERNAL_ERROR;
      my_error(err, MYF(0), "Innodb Clone Snapshot Invalid state");
      break;

    case CLONE_SNAPSHOT_FILE_COPY:
      ib::info(ER_IB_MSG_155) << "Clone State BEGIN FILE COPY";

      m_monitor.init_state(srv_stage_clone_file_copy.m_key, m_enable_pfs);
      err = init_file_copy();
      m_monitor.change_phase();
      DEBUG_SYNC_C("clone_start_page_archiving");
      DBUG_EXECUTE_IF("clone_crash_during_page_archiving", DBUG_SUICIDE(););
      break;

    case CLONE_SNAPSHOT_PAGE_COPY:
      ib::info(ER_IB_MSG_155) << "Clone State BEGIN PAGE COPY";

      m_monitor.init_state(srv_stage_clone_page_copy.m_key, m_enable_pfs);
      err = init_page_copy(temp_buffer, temp_buffer_len);
      m_monitor.change_phase();
      DEBUG_SYNC_C("clone_start_redo_archiving");
      break;

    case CLONE_SNAPSHOT_REDO_COPY:
      ib::info(ER_IB_MSG_155) << "Clone State BEGIN REDO COPY";

      m_monitor.init_state(srv_stage_clone_redo_copy.m_key, m_enable_pfs);
      err = init_redo_copy();
      m_monitor.change_phase();
      break;

    case CLONE_SNAPSHOT_DONE:
      ib::info(ER_IB_MSG_155) << "Clone State DONE ";

      m_monitor.init_state(PSI_NOT_INSTRUMENTED, m_enable_pfs);
      m_redo_ctx.release();
      break;
  }
  return (err);
}

Clone_File_Meta *Clone_Snapshot::get_file(Clone_File_Vec &file_vector,
                                          uint num_files, uint chunk_num,
                                          uint start_index) {
  Clone_File_Meta *current_file = nullptr;
  uint idx;

  ut_ad(file_vector.size() >= num_files);

  /* Scan through the file vector matching chunk number. */
  for (idx = start_index; idx < num_files; idx++) {
    current_file = file_vector[idx];

    ut_ad(chunk_num >= current_file->m_begin_chunk);

    if (chunk_num <= current_file->m_end_chunk) {
      break;
    }
  }

  ut_ad(idx < num_files);

  return (current_file);
}

int Clone_Snapshot::get_next_page(uint chunk_num, uint &block_num,
                                  Clone_File_Meta *file_meta,
                                  ib_uint64_t &data_offset, byte *&data_buf,
                                  uint &data_size) {
  Clone_Page clone_page;
  Clone_File_Meta *page_file;

  uint page_index;
  uint file_index;

  ut_ad(data_size >= UNIV_PAGE_SIZE);

  if (block_num == chunk_size()) {
    block_num = 0;
    return (0);
  }

  /* For "page copy", each block is a page. */
  page_index = chunk_size() * (chunk_num - 1);
  page_index += block_num;

  /* For last chunk, actual number of blocks could be less
  than chunk_size. */
  if (page_index >= m_page_vector.size()) {
    ut_ad(page_index == m_page_vector.size());
    block_num = 0;
    return (0);
  }

  clone_page = m_page_vector[page_index];

  ++block_num;

  /* Get the data file for current page. */
  bool found;
  const page_size_t &page_size =
      fil_space_get_page_size(clone_page.m_space_id, &found);

  ut_ad(found);

  file_index = m_data_file_map[clone_page.m_space_id];

  ut_ad(file_index > 0);
  --file_index;

  page_file = m_data_file_vector[file_index];
  ut_ad(page_file->m_space_id == clone_page.m_space_id);

  /* Data offset could be beyond 32 BIT integer. */
  data_offset = static_cast<ib_uint64_t>(clone_page.m_page_no);
  data_offset *= page_size.physical();

  /* Check if the page belongs to other nodes of the tablespace. */
  while (m_num_data_files > file_index + 1) {
    Clone_File_Meta *page_file_next;

    page_file_next = m_data_file_vector[file_index + 1];

    /* Next node belongs to same tablespace and data offset
    exceeds current node size */
    if (page_file_next->m_space_id == clone_page.m_space_id &&
        data_offset >= page_file->m_file_size) {
      data_offset -= page_file->m_file_size;
      file_index++;
      page_file = m_data_file_vector[file_index];
    } else {
      break;
    }
  }

  *file_meta = *page_file;

  /* Get page from buffer pool. */
  page_id_t page_id(clone_page.m_space_id, clone_page.m_page_no);

  auto err = get_page_for_write(page_id, page_size, data_buf, data_size);

  /* Update size from space header page. */
  if (clone_page.m_page_no == 0) {
    auto space_size = fsp_header_get_field(data_buf, FSP_SIZE);

    auto size_bytes = static_cast<uint64_t>(space_size);

    size_bytes *= page_size.physical();

    if (file_meta->m_file_size < size_bytes) {
      file_meta->m_file_size = size_bytes;
    }
  }
  return (err);
}



int Clone_Snapshot::init_file_copy() {
  int err = 0;

  ut_ad(m_snapshot_handle_type == CLONE_HDL_COPY);

  /* If not blocking clone, allocate redo header and trailer buffer. */
  if (m_snapshot_type != HA_CLONE_BLOCKING) {
    m_redo_ctx.get_header_size(m_redo_file_size, m_redo_header_size,
                               m_redo_trailer_size);

    m_redo_header = static_cast<byte *>(mem_heap_zalloc(
        m_snapshot_heap,
        m_redo_header_size + m_redo_trailer_size + UNIV_SECTOR_SIZE));

    if (m_redo_header == nullptr) {
      my_error(ER_OUTOFMEMORY, MYF(0),
               m_redo_header_size + m_redo_trailer_size);

      return (ER_OUTOFMEMORY);
    }

    m_redo_header =
        static_cast<byte *>(ut_align(m_redo_header, UNIV_SECTOR_SIZE));

    m_redo_trailer = m_redo_header + m_redo_header_size;
  }

  if (m_snapshot_type == HA_CLONE_REDO) {
    /* Start Redo Archiving */
    err = m_redo_ctx.start(m_redo_header, m_redo_header_size);

  } else if (m_snapshot_type == HA_CLONE_HYBRID ||
             m_snapshot_type == HA_CLONE_PAGE) {
    /* Start modified Page ID Archiving */
    err = m_page_ctx.start(false, nullptr);
  } else {
    ut_ad(m_snapshot_type == HA_CLONE_BLOCKING);
  }

  if (err != 0) {
    return (err);
  }

  /* Add buffer pool dump file. Always the first one in the list. */
  err = add_buf_pool_file();

  if (err != 0) {
    return (err);
  }

  /* Do not include redo files in file list. */
  bool include_log = (m_snapshot_type == HA_CLONE_BLOCKING);

  /* Iterate all tablespace files and add persistent data files. */
  auto error = Fil_iterator::for_each_file(
      include_log, [&](fil_node_t *file) { return (add_node(file)); });

  if (error != DB_SUCCESS) {
    return (ER_INTERNAL_ERROR);
  }

  ib::info(ER_IB_MSG_151) << "Clone State FILE COPY : " << m_num_current_chunks
                          << " chunks, "
                          << " chunk size : "
                          << (chunk_size() * UNIV_PAGE_SIZE) / (1024 * 1024)
                          << " M";

  return (0);
}

int Clone_Snapshot::init_page_copy(byte *page_buffer, uint page_buffer_len) {
  int err = 0;

  ut_ad(m_snapshot_handle_type == CLONE_HDL_COPY);

  if (m_snapshot_type == HA_CLONE_HYBRID) {
    /* Start Redo Archiving */
    err = m_redo_ctx.start(m_redo_header, m_redo_header_size);

  } else if (m_snapshot_type == HA_CLONE_PAGE) {
    /* Start COW for all modified pages - Not implemented. */
    ut_ad(false);
  } else {
    ut_ad(false);
  }

  if (err != 0) {
    goto func_end;
  }

  /* Stop modified page archiving. */
  err = m_page_ctx.stop(nullptr);

  DEBUG_SYNC_C("clone_stop_page_archiving_without_releasing");

  if (err != 0) {
    goto func_end;
  }

  /* Collect modified page Ids from Page Archiver. */
  void *context;
  uint aligned_size;

  context = static_cast<void *>(this);

  err = m_page_ctx.get_pages(add_page_callback, context, page_buffer,
                             page_buffer_len);

  m_page_vector.assign(m_page_set.begin(), m_page_set.end());

  aligned_size = ut_calc_align(m_num_pages, chunk_size());
  m_num_current_chunks = aligned_size >> m_chunk_size_pow2;

  ib::info(ER_IB_MSG_152) << "Clone State PAGE COPY : " << m_num_pages
                          << " pages, " << m_num_duplicate_pages
                          << " duplicate pages, " << m_num_current_chunks
                          << " chunks, "
                          << " chunk size : "
                          << (chunk_size() * UNIV_PAGE_SIZE) / (1024 * 1024)
                          << " M";

func_end:
  m_page_ctx.release();

  return (err);
}

int Clone_Snapshot::init_redo_copy() {
  ut_ad(m_snapshot_handle_type == CLONE_HDL_COPY);
  ut_ad(m_snapshot_type != HA_CLONE_BLOCKING);

  /* Stop redo archiving. */
  auto err = m_redo_ctx.stop(m_redo_trailer, m_redo_trailer_size,
                             m_redo_trailer_offset);

  if (err != 0) {
    return (err);
  }

  /* Collect archived redo log files from Log Archiver. */
  void *context;

  context = static_cast<void *>(this);

  err = m_redo_ctx.get_files(add_redo_file_callback, context);

  /* Add another chunk for the redo log header. */
  ++m_num_redo_chunks;

  m_monitor.add_estimate(m_redo_header_size);

  /* Add another chunk for the redo log trailer. */
  ++m_num_redo_chunks;

  if (m_redo_trailer_size != 0) {
    m_monitor.add_estimate(m_redo_trailer_size);
  }

  m_num_current_chunks = m_num_redo_chunks;

  ib::info(ER_IB_MSG_153) << "Clone State REDO COPY : " << m_num_current_chunks
                          << " chunks, "
                          << " chunk size : "
                          << (chunk_size() * UNIV_PAGE_SIZE) / (1024 * 1024)
                          << " M";

  return (err);
}

Clone_File_Meta *Clone_Snapshot::build_file(const char *file_name,
                                            ib_uint64_t file_size,
                                            ib_uint64_t file_offset,
                                            uint &num_chunks,
                                            bool copy_file_name) {
  Clone_File_Meta *file_meta;

  ib_uint64_t aligned_size;
  ib_uint64_t size_in_pages;

  /* Allocate for file metadata from snapshot heap. */
  aligned_size = sizeof(Clone_File_Meta);

  if (file_name != nullptr && copy_file_name) {
    aligned_size += strlen(file_name) + 1;
  }

  file_meta = static_cast<Clone_File_Meta *>(
      mem_heap_alloc(m_snapshot_heap, aligned_size));

  if (file_meta == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(0), static_cast<int>(aligned_size));
    return (file_meta);
  }

  /* For redo file with no data, add dummy entry. */
  if (file_name == nullptr) {
    num_chunks = 1;

    file_meta->m_file_name = nullptr;
    file_meta->m_file_name_len = 0;
    file_meta->m_file_size = 0;

    file_meta->m_begin_chunk = 1;
    file_meta->m_end_chunk = 1;

    return (file_meta);
  }

  file_meta->m_file_size = file_size;

  /* reduce offset amount from total size */
  ut_ad(file_size >= file_offset);
  file_size -= file_offset;

  /* Calculate and set chunk parameters. */
  size_in_pages = ut_uint64_align_up(file_size, UNIV_PAGE_SIZE);
  size_in_pages /= UNIV_PAGE_SIZE;

  aligned_size = ut_uint64_align_up(size_in_pages, chunk_size());

  num_chunks = static_cast<uint>(aligned_size >> m_chunk_size_pow2);

  file_meta->m_begin_chunk = m_num_current_chunks + 1;
  file_meta->m_end_chunk = m_num_current_chunks + num_chunks;

  file_meta->m_file_name_len = strlen(file_name) + 1;

  if (copy_file_name) {
    char *tmp_name = reinterpret_cast<char *>(file_meta + 1);

    strcpy(tmp_name, file_name);
    file_meta->m_file_name = const_cast<const char *>(tmp_name);
  } else {
    /* We use the same pointer as the tablespace and files
    should not be dropped or changed during clone. */
    file_meta->m_file_name = file_name;
  }

  return (file_meta);
}

int Clone_Snapshot::add_file(const char *name, ib_uint64_t size_bytes,
                             ulint space_id, bool copy_name) {
  ut_ad(m_snapshot_handle_type == CLONE_HDL_COPY);

  uint num_chunks;

  /* Build file metadata entry and add to data file vector. */
  auto file_meta = build_file(name, size_bytes, 0, num_chunks, copy_name);

  if (file_meta == nullptr) {
    return (ER_OUTOFMEMORY);
  }

  file_meta->m_space_id = space_id;

  file_meta->m_file_index = m_num_data_files;

  m_data_file_vector.push_back(file_meta);

  ++m_num_data_files;

  ut_ad(m_data_file_vector.size() == m_num_data_files);

  /* Update total number of chunks. */
  m_num_data_chunks += num_chunks;
  m_num_current_chunks = m_num_data_chunks;

  /* Update maximum file name length in snapshot. */
  if (file_meta->m_file_name_len > m_max_file_name_len) {
    m_max_file_name_len = static_cast<uint32_t>(file_meta->m_file_name_len);
  }

  return (0);
}

dberr_t Clone_Snapshot::add_node(fil_node_t *node) {
  ut_ad(m_snapshot_handle_type == CLONE_HDL_COPY);

  /* Exit if concurrent DDL in progress. */
  if (is_ddl_temp_table(node)) {
    my_error(ER_DDL_IN_PROGRESS, MYF(0));
    return (DB_ERROR);
  }

  /* Currently don't support encrypted tablespace. */
  auto space = node->space;
  if (space->encryption_type != Encryption::NONE) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0), "Clone Encrypted Tablespace");
    return (DB_ERROR);
  }

  /* Find out the file size from node. */
  page_size_t page_sz(space->flags);

  /* For compressed pages the file size doesn't match
  physical page size multiplied by number of pages. It is
  because we use UNIV_PAGE_SIZE while creating the node
  and tablespace. */

  uint64_t size_bytes;
  if (node->is_open && !page_sz.is_compressed()) {
    size_bytes = static_cast<ib_uint64_t>(node->size);
    size_bytes *= page_sz.physical();
  } else {
    os_file_size_t file_size;

    file_size = os_file_get_size(node->name);
    size_bytes = file_size.m_total_size;
  }

  m_monitor.add_estimate(size_bytes);

  /* Add file to snapshot. */
  auto err = add_file(node->name, size_bytes, space->id, false);

  if (err != 0) {
    return (DB_ERROR);
  }

  /* Add to hash map only for first node of the tablesapce. */
  if (m_data_file_map[space->id] == 0) {
    m_data_file_map[space->id] = m_num_data_files;
  }

  return (DB_SUCCESS);
}

int Clone_Snapshot::add_page(ib_uint32_t space_id, ib_uint32_t page_num) {
  Clone_Page cur_page;

  cur_page.m_space_id = space_id;
  cur_page.m_page_no = page_num;

  auto result = m_page_set.insert(cur_page);

  if (result.second) {
    m_num_pages++;
    m_monitor.add_estimate(UNIV_PAGE_SIZE);
  } else {
    m_num_duplicate_pages++;
  }

  return (0);
}

int Clone_Snapshot::add_redo_file(char *file_name, uint64_t file_size,
                                  uint64_t file_offset) {
  ut_ad(m_snapshot_handle_type == CLONE_HDL_COPY);

  Clone_File_Meta *file_meta;
  uint num_chunks;

  /* Build redo file metadata and add to redo vector. */
  file_meta = build_file(file_name, file_size, file_offset, num_chunks, true);

  m_monitor.add_estimate(file_meta->m_file_size - file_offset);

  if (file_meta == nullptr) {
    return (ER_OUTOFMEMORY);
  }

  /* Set the start offset for first redo file. This could happen
  if redo archiving was already in progress, possibly by another
  concurrent snapshot. */
  if (m_num_redo_files == 0) {
    m_redo_start_offset = file_offset;
  } else {
    ut_ad(file_offset == 0);
  }

  file_meta->m_space_id = dict_sys_t_s_log_space_first_id;

  file_meta->m_file_index = m_num_redo_files;

  m_redo_file_vector.push_back(file_meta);
  ++m_num_redo_files;

  ut_ad(m_redo_file_vector.size() == m_num_redo_files);

  m_num_redo_chunks += num_chunks;
  m_num_current_chunks = m_num_redo_chunks;

  return (0);
}


int Clone_Snapshot::get_file_from_desc(Clone_File_Meta *&file_desc,
                                       const char *data_dir, bool desc_create,
                                       bool &desc_exists) {
  int err = 0;

  mutex_enter(&m_snapshot_mutex);

  auto idx = file_desc->m_file_index;

  ut_ad(m_snapshot_handle_type == CLONE_HDL_APPLY);

  ut_ad(m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY ||
        m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY);

  Clone_File_Vec &file_vector = (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY)
                                    ? m_data_file_vector
                                    : m_redo_file_vector;

  desc_exists = false;

  /* File metadata is already there, possibly sent by another task. */
  if (file_vector[idx] != nullptr) {
    file_desc = file_vector[idx];
    desc_exists = true;

  } else if (desc_create) {
    /* Create the descriptor. */
    err = create_desc(data_dir, file_desc);
  }

  mutex_exit(&m_snapshot_mutex);

  return (err);
}

int Clone_Snapshot::create_desc(const char *data_dir,
                                Clone_File_Meta *&file_desc) {
  /* Build complete path for the new file to be added. */
  auto dir_len = static_cast<ulint>(strlen(data_dir));

  auto name_len = static_cast<ulint>((file_desc->m_file_name == nullptr)
                                         ? MAX_LOG_FILE_NAME
                                         : file_desc->m_file_name_len);

  auto alloc_size = static_cast<ulint>(dir_len + 1 + name_len);
  alloc_size += sizeof(Clone_File_Meta);

  auto ptr = static_cast<char *>(mem_heap_alloc(m_snapshot_heap, alloc_size));

  if (ptr == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(0), alloc_size);
    return (ER_OUTOFMEMORY);
  }

  auto file_meta = reinterpret_cast<Clone_File_Meta *>(ptr);
  ptr += sizeof(Clone_File_Meta);

  *file_meta = *file_desc;

  file_meta->m_file_name = static_cast<const char *>(ptr);
  name_len = 0;

  strcpy(ptr, data_dir);

  /* Add path separator at the end of data directory if not there. */
  if (ptr[dir_len - 1] != OS_PATH_SEPARATOR) {
    ptr[dir_len] = OS_PATH_SEPARATOR;
    ptr++;
    name_len++;
  }
  ptr += dir_len;
  name_len += dir_len;

  std::string name;
  char name_buf[MAX_LOG_FILE_NAME];
  bool absolute_path = false;

  /* Construct correct file path */
  if (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY) {
    name.assign(file_desc->m_file_name);
    absolute_path = Fil_path::is_absolute_path(name);

    if (absolute_path) {
      /* Set current pointer back as we don't want to append data directory
      for external files with absolute path. */
      ptr = const_cast<char *>(file_meta->m_file_name);
      name_len = 0;
    } else {
      /* For relative path remove "./" if there. */
      if (Fil_path::has_prefix(name, Fil_path::DOT_SLASH)) {
        name.erase(0, 2);
      }
    }
  } else {
    ut_ad(m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY);
    /* This is redo file. Use standard name. */
    snprintf(name_buf, MAX_LOG_FILE_NAME, "%s%u", ib_logfile_basename,
             file_desc->m_file_index);
    name.assign(name_buf);
  }

  strcpy(ptr, name.c_str());
  name_len += name.length();
  ++name_len;

  file_meta->m_file_name_len = name_len;
  file_desc = file_meta;

  /* For absolute path, we must ensure that the file is not
  present. This would always fail for local clone. */
  if (absolute_path) {
    ut_ad(m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY);

    auto is_hard_path = test_if_hard_path(file_desc->m_file_name);
    /* Check if the absolute path is not in right format */
    if (is_hard_path == 0) {
      my_error(ER_WRONG_VALUE, MYF(0), "file path", name.c_str());
      return (ER_WRONG_VALUE);
    }

    auto type = Fil_path::get_file_type(name);
    /* The file should not already exist */
    if (type == OS_FILE_TYPE_MISSING) {
      return (0);
    }
    if (type == OS_FILE_TYPE_FILE) {
      my_error(ER_FILE_EXISTS_ERROR, MYF(0), name.c_str());
      return (ER_FILE_EXISTS_ERROR);
    }
    /* Either the stat() call failed or the name is a
    directory/block device, or permission error etc. */
    char errbuf[MYSYS_STRERROR_SIZE];
    my_error(ER_ERROR_ON_WRITE, MYF(0), name.c_str(), errno,
             my_strerror(errbuf, sizeof(errbuf), errno));
    return (ER_ERROR_ON_WRITE);
  }
  return (0);
}


bool Clone_Snapshot::add_file_from_desc(Clone_File_Meta *&file_desc) {
  mutex_enter(&m_snapshot_mutex);

  ut_ad(m_snapshot_handle_type == CLONE_HDL_APPLY);

  if (m_snapshot_state == CLONE_SNAPSHOT_FILE_COPY) {
    m_data_file_vector[file_desc->m_file_index] = file_desc;
  } else {
    ut_ad(m_snapshot_state == CLONE_SNAPSHOT_REDO_COPY);
    m_redo_file_vector[file_desc->m_file_index] = file_desc;
  }

  mutex_exit(&m_snapshot_mutex);

  /** Check if it the last file */
  if (file_desc->m_file_index == m_num_data_files - 1) {
    return true;
  }

  return (false);
}

