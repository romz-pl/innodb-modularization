#include <innodb/log_recv/recv_sys_init.h>

#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/buf_pool/buf_pool_get_curr_size.h>
#include <innodb/disk/flags.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/log_recv/flags.h>
#include <innodb/log_recv/psf.h>
#include <innodb/log_recv/recv_max_page_lsn.h>
#include <innodb/log_recv/recv_n_pool_free_frames.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** Inits the recovery system for a recovery operation.
@param[in]	max_mem		Available memory in bytes */
void recv_sys_init(ulint max_mem) {
  if (recv_sys->spaces != nullptr) {
    return;
  }

  mutex_enter(&recv_sys->mutex);

#ifndef UNIV_HOTBACKUP
  if (!srv_read_only_mode) {
    recv_sys->flush_start = os_event_create(0);
    recv_sys->flush_end = os_event_create(0);
  }
#else  /* !UNIV_HOTBACKUP */
  recv_is_from_backup = true;
#endif /* !UNIV_HOTBACKUP */

  /* Set appropriate value of recv_n_pool_free_frames. If capacity
  is at least 10M and 25% above 512 pages then bump free frames to
  512. */
  if (buf_pool_get_curr_size() >= (10 * 1024 * 1024) &&
      (buf_pool_get_curr_size() >= ((512 + 128) * UNIV_PAGE_SIZE))) {
    /* Buffer pool of size greater than 10 MB. */
    recv_n_pool_free_frames = 512;
  }

  recv_sys->buf = static_cast<byte *>(ut_malloc_nokey(RECV_PARSING_BUF_SIZE));
  recv_sys->buf_len = RECV_PARSING_BUF_SIZE;

  recv_sys->len = 0;
  recv_sys->recovered_offset = 0;

  using Spaces = recv_sys_t::Spaces;

  recv_sys->spaces = UT_NEW(Spaces(), mem_log_recv_space_hash_key);

  recv_sys->n_addrs = 0;

  recv_sys->apply_log_recs = false;
  recv_sys->apply_batch_on = false;
  recv_sys->is_cloned_db = false;

  recv_sys->last_block_buf_start =
      static_cast<byte *>(ut_malloc_nokey(2 * OS_FILE_LOG_BLOCK_SIZE));

  recv_sys->last_block = static_cast<byte *>(
      ut_align(recv_sys->last_block_buf_start, OS_FILE_LOG_BLOCK_SIZE));

  recv_sys->found_corrupt_log = false;
  recv_sys->found_corrupt_fs = false;

  recv_max_page_lsn = 0;

  /* Call the constructor for both placement new objects. */
  new (&recv_sys->dblwr) recv_dblwr_t();

  new (&recv_sys->deleted) recv_sys_t::Missing_Ids();

  new (&recv_sys->missing_ids) recv_sys_t::Missing_Ids();

  recv_sys->metadata_recover = UT_NEW_NOKEY(MetadataRecover());

  mutex_exit(&recv_sys->mutex);
}
