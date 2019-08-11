#include <innodb/buf_chunk/buf_chunk_t.h>

#include <innodb/logger/warn.h>

/** Advices the OS that this chunk should not be dumped to a core file.
Emits a warning to the log if could not succeed.
@return true iff succeeded, false if no OS support or failed */
bool buf_chunk_t::madvise_dump() {
#ifdef HAVE_MADV_DONTDUMP
  if (madvise(mem, mem_size(), MADV_DODUMP)) {
    ib::warn(ER_IB_MSG_MADVISE_FAILED, mem, mem_size(), "MADV_DODUMP",
             strerror(errno));
    return false;
  }
  return true;
#else  /* HAVE_MADV_DONTDUMP */
  ib::warn(ER_IB_MSG_MADV_DONTDUMP_UNSUPPORTED);
  return false;
#endif /* HAVE_MADV_DONTDUMP */
}


/** Advices the OS that this chunk should be dumped to a core file.
Emits a warning to the log if could not succeed.
@return true iff succeeded, false if no OS support or failed */
bool buf_chunk_t::madvise_dont_dump() {
#ifdef HAVE_MADV_DONTDUMP
  if (madvise(mem, mem_size(), MADV_DONTDUMP)) {
    ib::warn(ER_IB_MSG_MADVISE_FAILED, mem, mem_size(), "MADV_DONTDUMP",
             strerror(errno));
    return false;
  }
  return true;
#else  /* HAVE_MADV_DONTDUMP */
  ib::warn(ER_IB_MSG_MADV_DONTDUMP_UNSUPPORTED);
  return false;
#endif /* HAVE_MADV_DONTDUMP */
}

