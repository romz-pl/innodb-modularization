#include <innodb/sync_rw/rw_lock_debug_print.h>

#ifdef UNIV_DEBUG

/** Prints info of a debug struct. */
void rw_lock_debug_print(FILE *f,                     /*!< in: output stream */
                         const rw_lock_debug_t *info) /*!< in: debug struct */
{
  ulint rwt = info->lock_type;

  fprintf(f, "Locked: thread " UINT64PF " file %s line " ULINTPF "  ",
          (uint64_t)(info->thread_id), sync_basename(info->file_name),
          info->line);

  switch (rwt) {
    case RW_LOCK_S:
      fputs("S-LOCK", f);
      break;
    case RW_LOCK_X:
      fputs("X-LOCK", f);
      break;
    case RW_LOCK_SX:
      fputs("SX-LOCK", f);
      break;
    case RW_LOCK_X_WAIT:
      fputs("WAIT X-LOCK", f);
      break;
    default:
      ut_error;
  }

  if (info->pass != 0) {
    fprintf(f, " pass value %lu", (ulong)info->pass);
  }

  fprintf(f, "\n");
}

#endif
