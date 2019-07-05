#pragma once

#include <innodb/univ/univ.h>

#include <innodb/ioasync/AIO_mode.h>
#include <innodb/ioasync/Slot.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/sync_event/os_event_wait.h>
#include <innodb/sync_latch/latch_id_t.h>
#include <innodb/sync_mutex/SysMutex.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

#include <vector>

/** The asynchronous i/o array structure */
class AIO {
 public:
  /** Constructor
  @param[in]	id		Latch ID
  @param[in]	n		Number of slots to configure
  @param[in]	segments	Number of segments to configure */
  AIO(latch_id_t id, ulint n, ulint segments);

  /** Destructor */
  ~AIO();

  /** Initialize the instance
  @return DB_SUCCESS or error code */
  dberr_t init();

  /** Requests for a slot in the aio array. If no slot is available, waits
  until not_full-event becomes signaled.

  @param[in,out]	type	IO context
  @param[in,out]	m1	message to be passed along with the AIO
                          operation
  @param[in,out]	m2	message to be passed along with the AIO
                          operation
  @param[in]	file	file handle
  @param[in]	name	name of the file or path as a null-terminated
                          string
  @param[in,out]	buf	buffer where to read or from which to write
  @param[in]	offset	file offset, where to read from or start writing
  @param[in]	len	length of the block to read or write
  @return pointer to slot */
  Slot *reserve_slot(IORequest &type, fil_node_t *m1, void *m2,
                     pfs_os_file_t file, const char *name, void *buf,
                     os_offset_t offset, ulint len)
      MY_ATTRIBUTE((warn_unused_result));

  /** @return number of reserved slots */
  ulint pending_io_count() const;

  /** Returns a pointer to the nth slot in the aio array.
  @param[in]	i	Index of the slot in the array
  @return pointer to slot */
  const Slot *at(ulint i) const MY_ATTRIBUTE((warn_unused_result)) {
    ut_a(i < m_slots.size());

    return (&m_slots[i]);
  }

  /** Non const version */
  Slot *at(ulint i) MY_ATTRIBUTE((warn_unused_result)) {
    ut_a(i < m_slots.size());

    return (&m_slots[i]);
  }

  /** Frees a slot in the AIO array, assumes caller owns the mutex.
  @param[in,out]	slot	Slot to release */
  void release(Slot *slot);

  /** Frees a slot in the AIO array, assumes caller doesn't own the mutex.
  @param[in,out]	slot	Slot to release */
  void release_with_mutex(Slot *slot);

  /** Prints info about the aio array.
  @param[in,out]	file	Where to print */
  void print(FILE *file);

  /** @return the number of slots per segment */
  ulint slots_per_segment() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_slots.size() / m_n_segments);
  }

  /** @return accessor for n_segments */
  ulint get_n_segments() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_n_segments);
  }

#ifdef UNIV_DEBUG
  /** @return true if the thread owns the mutex */
  bool is_mutex_owned() const MY_ATTRIBUTE((warn_unused_result)) {
    return (mutex_own(&m_mutex));
  }
#endif /* UNIV_DEBUG */

  /** Acquire the mutex */
  void acquire() const { mutex_enter(&m_mutex); }

  /** Release the mutex */
  void release() const { mutex_exit(&m_mutex); }

  /** Write out the state to the file/stream
  @param[in, out]	file	File to write to */
  void to_file(FILE *file) const;

#ifdef LINUX_NATIVE_AIO
  /** Dispatch an AIO request to the kernel.
  @param[in,out]	slot	an already reserved slot
  @return true on success. */
  bool linux_dispatch(Slot *slot) MY_ATTRIBUTE((warn_unused_result));

  /** Accessor for an AIO event
  @param[in]	index	Index into the array
  @return the event at the index */
  io_event *io_events(ulint index) MY_ATTRIBUTE((warn_unused_result)) {
    ut_a(index < m_events.size());

    return (&m_events[index]);
  }

  /** Accessor for the AIO context
  @param[in]	segment	Segment for which to get the context
  @return the AIO context for the segment */
  io_context *io_ctx(ulint segment) MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(segment < get_n_segments());

    return (m_aio_ctx[segment]);
  }

  /** Creates an io_context for native linux AIO.
  @param[in]	max_events	number of events
  @param[out]	io_ctx		io_ctx to initialize.
  @return true on success. */
  static bool linux_create_io_ctx(ulint max_events, io_context_t *io_ctx)
      MY_ATTRIBUTE((warn_unused_result));

  /** Checks if the system supports native linux aio. On some kernel
  versions where native aio is supported it won't work on tmpfs. In such
  cases we can't use native aio as it is not possible to mix simulated
  and native aio.
  @return true if supported, false otherwise. */
  static bool is_linux_native_aio_supported()
      MY_ATTRIBUTE((warn_unused_result));
#endif /* LINUX_NATIVE_AIO */

#ifdef WIN_ASYNC_IO
  /** Wakes up all async i/o threads in the array in Windows async I/O at
  shutdown. */
  void signal() {
    for (ulint i = 0; i < m_slots.size(); ++i) {
      SetEvent(m_slots[i].handle);
    }
  }

  /** Wake up all AIO threads in Windows native aio */
  static void wake_at_shutdown() {
    s_reads->signal();

    if (s_writes != NULL) {
      s_writes->signal();
    }

    if (s_ibuf != NULL) {
      s_ibuf->signal();
    }

    if (s_log != NULL) {
      s_log->signal();
    }
  }
#endif /* WIN_ASYNC_IO */

#ifdef _WIN32
  /** This function can be called if one wants to post a batch of reads
  and prefers an I/O - handler thread to handle them all at once later.You
  must call os_aio_simulated_wake_handler_threads later to ensure the
  threads are not left sleeping! */
  static void simulated_put_read_threads_to_sleep();

  /** The non asynchronous IO array.
  @return the synchronous AIO array instance. */
  static AIO *sync_array() MY_ATTRIBUTE((warn_unused_result)) {
    return (s_sync);
  }

  /**
  Get the AIO handles for a segment.
  @param[in]	segment		The local segment.
  @return the handles for the segment. */
  HANDLE *handles(ulint segment) MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(segment < m_handles->size() / slots_per_segment());

    return (&(*m_handles)[segment * slots_per_segment()]);
  }

  /** @return true if no slots are reserved */
  bool is_empty() const MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(is_mutex_owned());
    return (m_n_reserved == 0);
  }
#endif /* _WIN32 */

  /** Create an instance using new(std::nothrow)
  @param[in]	id		Latch ID
  @param[in]	n		The number of AIO request slots
  @param[in]	n_segments	The number of segments
  @return a new AIO instance */
  static AIO *create(latch_id_t id, ulint n, ulint n_segments)
      MY_ATTRIBUTE((warn_unused_result));

  /** Initializes the asynchronous io system. Creates one array each
  for ibuf and log I/O. Also creates one array each for read and write
  where each array is divided logically into n_readers and n_writers
  respectively. The caller must create an i/o handler thread for each
  segment in these arrays. This function also creates the sync array.
  No I/O handler thread needs to be created for that
  @param[in]	n_per_seg	maximum number of pending aio
                                  operations allowed per segment
  @param[in]	n_readers	number of reader threads
  @param[in]	n_writers	number of writer threads
  @param[in]	n_slots_sync	number of slots in the sync aio array
  @return true if AIO sub-system was started successfully */
  static bool start(ulint n_per_seg, ulint n_readers, ulint n_writers,
                    ulint n_slots_sync) MY_ATTRIBUTE((warn_unused_result));

  /** Free the AIO arrays */
  static void shutdown();

  /** Print all the AIO segments
  @param[in,out]	file		Where to print */
  static void print_all(FILE *file);

  /** Calculates local segment number and aio array from global
  segment number.
  @param[out]	array		AIO wait array
  @param[in]	segment		global segment number
  @return local segment number within the aio array */
  static ulint get_array_and_local_segment(AIO **array, ulint segment)
      MY_ATTRIBUTE((warn_unused_result));

  /** Select the IO slot array
  @param[in,out]	type		Type of IO, READ or WRITE
  @param[in]	read_only	true if running in read-only mode
  @param[in]	aio_mode	IO mode
  @return slot array or NULL if invalid mode specified */
  static AIO *select_slot_array(IORequest &type, bool read_only,
                                AIO_mode aio_mode)
      MY_ATTRIBUTE((warn_unused_result));

  /** Calculates segment number for a slot.
  @param[in]	array		AIO wait array
  @param[in]	slot		slot in this array
  @return segment number (which is the number used by, for example,
          I/O handler threads) */
  static ulint get_segment_no_from_slot(const AIO *array, const Slot *slot)
      MY_ATTRIBUTE((warn_unused_result));

  /** Wakes up a simulated AIO I/O-handler thread if it has something
  to do.
  @param[in]	global_segment	the number of the segment in the
                                  AIO arrays */
  static void wake_simulated_handler_thread(ulint global_segment);

  /** Check if it is a read request
  @param[in]	aio		The AIO instance to check
  @return true if the AIO instance is for reading. */
  static bool is_read(const AIO *aio) MY_ATTRIBUTE((warn_unused_result)) {
    return (s_reads == aio);
  }

  /** Wait on an event until no pending writes */
  static void wait_until_no_pending_writes() {
    os_event_wait(AIO::s_writes->m_is_empty);
  }

  /** Print to file
  @param[in]	file		File to write to */
  static void print_to_file(FILE *file);

  /** Check for pending IO. Gets the count and also validates the
  data structures.
  @return count of pending IO requests */
  static ulint total_pending_io_count();

 private:
  /** Initialise the slots
  @return DB_SUCCESS or error code */
  dberr_t init_slots() MY_ATTRIBUTE((warn_unused_result));

  /** Wakes up a simulated AIO I/O-handler thread if it has something
  to do for a local segment in the AIO array.
  @param[in]	global_segment	the number of the segment in the
                                  AIO arrays
  @param[in]	segment		the local segment in the AIO array */
  void wake_simulated_handler_thread(ulint global_segment, ulint segment);

  /** Prints pending IO requests per segment of an aio array.
  We probably don't need per segment statistics but they can help us
  during development phase to see if the IO requests are being
  distributed as expected.
  @param[in,out]	file		file where to print
  @param[in]	segments	pending IO array */
  void print_segment_info(FILE *file, const ulint *segments);

#ifdef LINUX_NATIVE_AIO
  /** Initialise the Linux native AIO data structures
  @return DB_SUCCESS or error code */
  dberr_t init_linux_native_aio() MY_ATTRIBUTE((warn_unused_result));
#endif /* LINUX_NATIVE_AIO */

 private:
  typedef std::vector<Slot> Slots;

  /** the mutex protecting the aio array */
  mutable SysMutex m_mutex;

  /** Pointer to the slots in the array.
  Number of elements must be divisible by n_threads. */
  Slots m_slots;

  /** Number of segments in the aio array of pending aio requests.
  A thread can wait separately for any one of the segments. */
  ulint m_n_segments;

  /** The event which is set to the signaled state when
  there is space in the aio outside the ibuf segment */
  os_event_t m_not_full;

  /** The event which is set to the signaled state when
  there are no pending i/os in this array */
  os_event_t m_is_empty;

  /** Number of reserved slots in the AIO array outside
  the ibuf segment */
  ulint m_n_reserved;

#ifdef _WIN32
  typedef std::vector<HANDLE, ut_allocator<HANDLE>> Handles;

  /** Pointer to an array of OS native event handles where
  we copied the handles from slots, in the same order. This
  can be used in WaitForMultipleObjects; used only in Windows */
  Handles *m_handles;
#endif /* _WIN32 */

#if defined(LINUX_NATIVE_AIO)
  typedef std::vector<io_event> IOEvents;

  /** completion queue for IO. There is one such queue per
  segment. Each thread will work on one ctx exclusively. */
  io_context_t *m_aio_ctx;

  /** The array to collect completed IOs. There is one such
  event for each possible pending IO. The size of the array
  is equal to m_slots.size(). */
  IOEvents m_events;
#endif /* LINUX_NATIV_AIO */

  /** The aio arrays for non-ibuf i/o and ibuf i/o, as well as
  sync AIO. These are NULL when the module has not yet been
  initialized. */

  /** Insert buffer */
  static AIO *s_ibuf;

  /** Redo log */
  static AIO *s_log;

  /** Reads */
  static AIO *s_reads;

  /** Writes */
  static AIO *s_writes;

  /** Synchronous I/O */
  static AIO *s_sync;
};
