#pragma once

#include <innodb/univ/univ.h>

#if defined(LINUX_NATIVE_AIO)

#include <innodb/assert/assert.h>
#include <innodb/error/dberr_t.h>
#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/ioasync/AIO.h>
#include <innodb/ioasync/buf_page_cleaner_is_active.h>

struct Slot;
class IORequest;
struct fil_node_t;

/** At a shutdown this value climbs from SRV_SHUTDOWN_NONE to
SRV_SHUTDOWN_CLEANUP and then to SRV_SHUTDOWN_LAST_PHASE, and so on */
extern enum srv_shutdown_t srv_shutdown_state;


/** Linux native AIO handler */
class LinuxAIOHandler {
 public:
  /**
  @param[in] global_segment	The global segment*/
  LinuxAIOHandler(ulint global_segment) : m_global_segment(global_segment) {
    /* Should never be doing Sync IO here. */
    ut_a(m_global_segment != ULINT_UNDEFINED);

    /* Find the array and the local segment. */

    m_segment = AIO::get_array_and_local_segment(&m_array, m_global_segment);

    m_n_slots = m_array->slots_per_segment();
  }

  /** Destructor */
  ~LinuxAIOHandler() {
    // No op
  }

  /**
  Process a Linux AIO request
  @param[out]	m1		the messages passed with the
  @param[out]	m2		AIO request; note that in case the
                                  AIO operation failed, these output
                                  parameters are valid and can be used to
                                  restart the operation.
  @param[out]	request		IO context
  @return DB_SUCCESS or error code */
  dberr_t poll(fil_node_t **m1, void **m2, IORequest *request);

 private:
  /** Resubmit an IO request that was only partially successful
  @param[in,out]	slot		Request to resubmit
  @return DB_SUCCESS or DB_FAIL if the IO resubmit request failed */
  dberr_t resubmit(Slot *slot);

  /** Check if the AIO succeeded
  @param[in,out]	slot		The slot to check
  @return DB_SUCCESS, DB_FAIL if the operation should be retried or
          DB_IO_ERROR on all other errors */
  dberr_t check_state(Slot *slot);

  /** @return true if a shutdown was detected */
  bool is_shutdown() const {
    return (srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS &&
            !buf_page_cleaner_is_active);
  }

  /** If no slot was found then the m_array->m_mutex will be released.
  @param[out]	n_pending	The number of pending IOs
  @return NULL or a slot that has completed IO */
  Slot *find_completed_slot(ulint *n_pending);

  /** This is called from within the IO-thread. If there are no completed
  IO requests in the slot array, the thread calls this function to
  collect more requests from the Linux kernel.
  The IO-thread waits on io_getevents(), which is a blocking call, with
  a timeout value. Unless the system is very heavy loaded, keeping the
  IO-thread very busy, the io-thread will spend most of its time waiting
  in this function.
  The IO-thread also exits in this function. It checks server status at
  each wakeup and that is why we use timed wait in io_getevents(). */
  void collect();

 private:
  /** Slot array */
  AIO *m_array;

  /** Number of slots inthe local segment */
  ulint m_n_slots;

  /** The local segment to check */
  ulint m_segment;

  /** The global segment */
  ulint m_global_segment;
};


#endif
