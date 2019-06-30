#include <innodb/sync_array/sync_array_cell_print.h>
#include <innodb/formatting/formatting.h>
#include <innodb/sync_rw/rw_lock_get_writer.h>
#include <innodb/error/ut_error.h>
#include <innodb/sync_rw/rw_lock_get_reader_count.h>
#include <innodb/string/innobase_basename.h>


/** Reports info of a wait array cell. */
void sync_array_cell_print(FILE *file, /*!< in: file where to print */
                                  sync_cell_t *cell) /*!< in: sync cell */
{
  rw_lock_t *rwlock;
  ulint type;
  ulint writer;

  type = cell->request_type;

  fprintf(file,
          "--Thread " UINT64PF " has waited at %s line " ULINTPF
          " for %.2f seconds the semaphore:\n",
          (uint64_t)(cell->thread_id), innobase_basename(cell->file),
          cell->line, difftime(time(NULL), cell->reservation_time));

  if (type == SYNC_MUTEX) {
    WaitMutex *mutex = cell->latch.mutex;
    const WaitMutex::MutexPolicy &policy = mutex->policy();
#ifdef UNIV_DEBUG
    const char *name = policy.get_enter_filename();
    if (name == NULL) {
      /* The mutex might have been released. */
      name = "NULL";
    }
#endif /* UNIV_DEBUG */

    fprintf(file,
            "Mutex at %p, %s, lock var %lu\n"
#ifdef UNIV_DEBUG
            "Last time reserved in file %s line %lu"
#endif /* UNIV_DEBUG */
            "\n",
            (void *)mutex, policy.to_string().c_str(), (ulong)mutex->state()
#ifdef UNIV_DEBUG
                                                           ,
            name, (ulong)policy.get_enter_line()
#endif /* UNIV_DEBUG */
    );
  } else if (type == SYNC_BUF_BLOCK) {
    BlockWaitMutex *mutex = cell->latch.bpmutex;

    const BlockWaitMutex::MutexPolicy &policy = mutex->policy();
#ifdef UNIV_DEBUG
    const char *name = policy.get_enter_filename();
    if (name == NULL) {
      /* The mutex might have been released. */
      name = "NULL";
    }
#endif /* UNIV_DEBUG */

    fprintf(file,
            "Mutex at %p, %s, lock var %lu\n"
#ifdef UNIV_DEBUG
            "Last time reserved in file %s line %lu"
#endif /* UNIV_DEBUG */
            "\n",
            (void *)mutex, policy.to_string().c_str(), (ulong)mutex->state()
#ifdef UNIV_DEBUG
                                                           ,
            name, (ulong)policy.get_enter_line()
#endif /* UNIV_DEBUG */
    );
  } else if (type == RW_LOCK_X || type == RW_LOCK_X_WAIT ||
             type == RW_LOCK_SX || type == RW_LOCK_S) {
    fputs(type == RW_LOCK_X
              ? "X-lock on"
              : type == RW_LOCK_X_WAIT
                    ? "X-lock (wait_ex) on"
                    : type == RW_LOCK_SX ? "SX-lock on" : "S-lock on",
          file);

    rwlock = cell->latch.lock;

    fprintf(file, " RW-latch at %p created in file %s line %lu\n",
            (void *)rwlock, innobase_basename(rwlock->cfile_name),
            (ulong)rwlock->cline);

    writer = rw_lock_get_writer(rwlock);

    if (writer != RW_LOCK_NOT_LOCKED) {
      fprintf(file,
              "a writer (thread id " UINT64PF
              ") has"
              " reserved it in mode %s",
              (uint64_t)(rwlock->writer_thread),
              writer == RW_LOCK_X
                  ? " exclusive\n"
                  : writer == RW_LOCK_SX ? " SX\n" : " wait exclusive\n");
    }

    fprintf(file,
            "number of readers " ULINTPF ", waiters flag " ULINTPF
            ", lock_word: %lx\n"
            "Last time read locked in file %s line %lu\n"
            "Last time write locked in file %s line %lu\n",
            rw_lock_get_reader_count(rwlock), rwlock->waiters,
            static_cast<ulong>(rwlock->lock_word),
            innobase_basename(rwlock->last_s_file_name),
            static_cast<ulong>(rwlock->last_s_line), rwlock->last_x_file_name,
            static_cast<ulong>(rwlock->last_x_line));
  } else {
    ut_error;
  }

  if (!cell->waiting) {
    fputs("wait has ended\n", file);
  }
}

