#include <innodb/buffer/buf_pool_t.h>

#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/buffer/buf_flush_list_mutex_own.h>

#include <ostream>

/** Print the given buf_pool_t object.
@param[in,out]	out		the output stream
@param[in]	buf_pool	the buf_pool_t object to be printed
@return the output stream */
std::ostream &operator<<(std::ostream &out, const buf_pool_t &buf_pool) {
#ifndef UNIV_HOTBACKUP
  /* These locking requirements might be relaxed if desired */
  ut_ad(mutex_own(&buf_pool.LRU_list_mutex));
  ut_ad(mutex_own(&buf_pool.free_list_mutex));
  ut_ad(mutex_own(&buf_pool.flush_state_mutex));
  ut_ad(buf_flush_list_mutex_own(&buf_pool));

  out << "[buffer pool instance: "
      << "buf_pool size=" << buf_pool.curr_size
      << ", database pages=" << UT_LIST_GET_LEN(buf_pool.LRU)
      << ", free pages=" << UT_LIST_GET_LEN(buf_pool.free)
      << ", modified database pages=" << UT_LIST_GET_LEN(buf_pool.flush_list)
      << ", n pending decompressions=" << buf_pool.n_pend_unzip
      << ", n pending reads=" << buf_pool.n_pend_reads
      << ", n pending flush LRU=" << buf_pool.n_flush[BUF_FLUSH_LRU]
      << " list=" << buf_pool.n_flush[BUF_FLUSH_LIST]
      << " single page=" << buf_pool.n_flush[BUF_FLUSH_SINGLE_PAGE]
      << ", pages made young=" << buf_pool.stat.n_pages_made_young
      << ", not young=" << buf_pool.stat.n_pages_not_made_young
      << ", pages read=" << buf_pool.stat.n_pages_read
      << ", created=" << buf_pool.stat.n_pages_created
      << ", written=" << buf_pool.stat.n_pages_written << "]";
#endif /* !UNIV_HOTBACKUP */
  return (out);
}

