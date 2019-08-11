#include <innodb/buf_pool/buf_pool_t.h>

#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/buf_flush/buf_flush_list_mutex_own.h>
#include <innodb/buffer/innobase_disable_core_dump.h>
#include <innodb/buf_chunk/buf_chunk_t.h>
#include <innodb/buf_pool/buf_pool_should_madvise.h>

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


/** A wrapper for buf_pool_t::allocator.alocate_large which also advices the OS
that this chunk should not be dumped to a core file if that was requested.
Emits a warning to the log and disables @@global.core_file if advising was
requested but could not be performed, but still return true as the allocation
itself succeeded.
@param[in]	mem_size  number of bytes to allocate
@param[in,out]  chunk     mem and mem_pfx fields of this chunk will be updated
                          to contain information about allocated memory region
@return true iff allocated successfully */
bool buf_pool_t::allocate_chunk(ulonglong mem_size, buf_chunk_t *chunk) {
  ut_ad(mutex_own(&chunks_mutex));
  chunk->mem = allocator.allocate_large(mem_size, &chunk->mem_pfx);
  if (chunk->mem == NULL) {
    return false;
  }
  /* Dump core without large memory buffers */
  if (buf_pool_should_madvise) {
    if (!chunk->madvise_dont_dump()) {
      innobase_disable_core_dump();
    }
  }
  return true;
}

/** A wrapper for buf_pool_t::allocator.deallocate_large which also advices the
OS that this chunk can be dumped to a core file.
Emits a warning to the log and disables @@global.core_file if advising was
requested but could not be performed.
@param[in]  chunk    mem and mem_pfx fields of this chunk will be used to locate
                     the memory region to free */
void buf_pool_t::deallocate_chunk(buf_chunk_t *chunk) {
  ut_ad(mutex_own(&chunks_mutex));
  /* Undo the effect of the earlier MADV_DONTDUMP */
  if (buf_pool_should_madvise) {
    if (!chunk->madvise_dump()) {
      innobase_disable_core_dump();
    }
  }
  allocator.deallocate_large(chunk->mem, &chunk->mem_pfx);
}

/** Advices the OS that all chunks in this buffer pool instance can be dumped
to a core file.
Emits a warning to the log if could not succeed.
@return true iff succeeded, false if no OS support or failed */
bool buf_pool_t::madvise_dump() {
  ut_ad(mutex_own(&chunks_mutex));
  for (buf_chunk_t *chunk = chunks; chunk < chunks + n_chunks; chunk++) {
    if (!chunk->madvise_dump()) {
      return false;
    }
  }
  return true;
}

/** Advices the OS that all chunks in this buffer pool instance should not
be dumped to a core file.
Emits a warning to the log if could not succeed.
@return true iff succeeded, false if no OS support or failed */
bool buf_pool_t::madvise_dont_dump() {
  ut_ad(mutex_own(&chunks_mutex));
  for (buf_chunk_t *chunk = chunks; chunk < chunks + n_chunks; chunk++) {
    if (!chunk->madvise_dont_dump()) {
      return false;
    }
  }
  return true;
}
