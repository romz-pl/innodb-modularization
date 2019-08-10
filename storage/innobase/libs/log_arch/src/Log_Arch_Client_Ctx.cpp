#include <innodb/log_arch/Log_Arch_Client_Ctx.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/disk/flags.h>
#include <innodb/log_arch/Arch_Group.h>
#include <innodb/log_arch/arch_log_sys.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/srv_log_file_size.h>
#include <innodb/logger/info.h>

/** Get redo file, header and trailer size
@param[out]	file_sz		redo file size
@param[out]	header_sz	redo header size
@param[out]	trailer_sz	redo trailer size */
void Log_Arch_Client_Ctx::get_header_size(ib_uint64_t &file_sz, uint &header_sz,
                                          uint &trailer_sz) {
  file_sz = srv_log_file_size;
  header_sz = LOG_FILE_HDR_SIZE;
  trailer_sz = OS_FILE_LOG_BLOCK_SIZE;
}

/** Start redo log archiving
@param[out]	header	redo header. Caller must allocate buffer.
@param[in]	len	buffer length
@return error code */
int Log_Arch_Client_Ctx::start(byte *header, uint len) {
  ut_ad(len >= LOG_FILE_HDR_SIZE);

  auto err = arch_log_sys->start(m_group, m_begin_lsn, header, false);

  if (err != 0) {
    return (err);
  }

  m_state = ARCH_CLIENT_STATE_STARTED;

  ib::info(ER_IB_MSG_15) << "Clone Start LOG ARCH : start LSN : "
                         << m_begin_lsn;

  return (0);
}

/** Stop redo log archiving. Exact trailer length is returned as out
parameter which could be less than the redo block size.
@param[out]	trailer	redo trailer. Caller must allocate buffer.
@param[in,out]	len	trailer length
@param[out]	offset	trailer block offset
@return error code */
int Log_Arch_Client_Ctx::stop(byte *trailer, uint32_t &len, uint64_t &offset) {
  lsn_t start_lsn;
  lsn_t stop_lsn;

  ut_ad(m_state == ARCH_CLIENT_STATE_STARTED);
  ut_ad(trailer == nullptr || len >= OS_FILE_LOG_BLOCK_SIZE);

  auto err = arch_log_sys->stop(m_group, m_end_lsn, trailer, len);

  start_lsn = m_group->get_begin_lsn();

  start_lsn = ut_uint64_align_down(start_lsn, OS_FILE_LOG_BLOCK_SIZE);
  stop_lsn = ut_uint64_align_down(m_end_lsn, OS_FILE_LOG_BLOCK_SIZE);

  lsn_t file_capacity = m_group->get_file_size();

  file_capacity -= LOG_FILE_HDR_SIZE;

  offset = (stop_lsn - start_lsn) % file_capacity;

  offset += LOG_FILE_HDR_SIZE;

  m_state = ARCH_CLIENT_STATE_STOPPED;

  ib::info(ER_IB_MSG_16) << "Clone Stop  LOG ARCH : end LSN : " << m_end_lsn;

  return (err);
}

/** Get archived data file details
@param[in]	cbk_func	callback called for each file
@param[in]	ctx		callback function context
@return error code */
int Log_Arch_Client_Ctx::get_files(Log_Arch_Cbk *cbk_func, void *ctx) {
  ut_ad(m_state == ARCH_CLIENT_STATE_STOPPED);

  int err = 0;

  auto size = m_group->get_file_size();

  /* Check if the archived redo log is less than one block size. In this
  case we send the data in trailer buffer. */
  auto low_begin = ut_uint64_align_down(m_begin_lsn, OS_FILE_LOG_BLOCK_SIZE);

  auto low_end = ut_uint64_align_down(m_end_lsn, OS_FILE_LOG_BLOCK_SIZE);

  if (low_begin == low_end) {
    err = cbk_func(nullptr, size, 0, ctx);
    return (err);
  }

  /* Get the start lsn of the group */
  auto start_lsn = m_group->get_begin_lsn();
  start_lsn = ut_uint64_align_down(start_lsn, OS_FILE_LOG_BLOCK_SIZE);

  ut_ad(m_begin_lsn >= start_lsn);

  /* Calculate first file index and offset for this client. */
  lsn_t lsn_diff = m_begin_lsn - start_lsn;
  uint64_t capacity = size - LOG_FILE_HDR_SIZE;

  auto idx = static_cast<uint>(lsn_diff / capacity);
  uint64_t offset = lsn_diff % capacity;

  /* Set start lsn to the beginning of file. */
  start_lsn = m_begin_lsn - offset;

  offset += LOG_FILE_HDR_SIZE;
  offset = ut_uint64_align_down(offset, OS_FILE_LOG_BLOCK_SIZE);

  /* Callback with all archive file names that holds the range of log
  data for this client. */
  while (start_lsn < m_end_lsn) {
    char name[MAX_ARCH_LOG_FILE_NAME_LEN];
    m_group->get_file_name(idx, name, MAX_ARCH_LOG_FILE_NAME_LEN);

    idx++;
    start_lsn += capacity;

    /* For last file adjust the size based on end lsn. */
    if (start_lsn >= m_end_lsn) {
      lsn_diff =
          ut_uint64_align_up(start_lsn - m_end_lsn, OS_FILE_LOG_BLOCK_SIZE);
      size -= lsn_diff;
    }

    err = cbk_func(name, size, offset, ctx);

    if (err != 0) {
      break;
    }

    /* offset could be non-zero only for first file. */
    offset = 0;
  }

  return (err);
}

/** Release archived data so that system can purge it */
void Log_Arch_Client_Ctx::release() {
  if (m_state == ARCH_CLIENT_STATE_INIT) {
    return;
  }

  if (m_state == ARCH_CLIENT_STATE_STARTED) {
    uint64_t dummy_offset;
    uint32_t dummy_len = 0;

    /* This is for cleanup in error cases. */
    stop(nullptr, dummy_len, dummy_offset);
  }

  ut_ad(m_state == ARCH_CLIENT_STATE_STOPPED);

  arch_log_sys->release(m_group, false);

  m_group = nullptr;

  m_begin_lsn = LSN_MAX;
  m_end_lsn = LSN_MAX;

  m_state = ARCH_CLIENT_STATE_INIT;
}
