#include <innodb/ioasync/AIO.h>

#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/disk/flags.h>
#include <innodb/error/ut_error.h>
#include <innodb/formatting/formatting.h>
#include <innodb/io/os_file_compress_page.h>
#include <innodb/io/os_file_encrypt_log.h>
#include <innodb/io/os_file_encrypt_page.h>
#include <innodb/io/os_free_block.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/io/srv_use_native_aio.h>
#include <innodb/ioasync/SRV_MAX_N_IO_THREADS.h>
#include <innodb/ioasync/os_aio_n_segments.h>
#include <innodb/ioasync/os_aio_segment_wait_events.h>
#include <innodb/ioasync/os_aio_simulated_wake_handler_threads.h>
#include <innodb/ioasync/os_aio_validate.h>
#include <innodb/ioasync/os_last_printout.h>
#include <innodb/ioasync/srv_io_thread_function.h>
#include <innodb/ioasync/srv_reset_io_thread_op_info.h>
#include <innodb/logger/info.h>
#include <innodb/logger/warn.h>
#include <innodb/sync_event/os_event_create.h>
#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_set.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_mutex/mutex_destroy.h>
#include <innodb/time/ut_time.h>

/** time to sleep, in microseconds if io_setup() returns EAGAIN. */
static const ulint OS_AIO_IO_SETUP_RETRY_SLEEP = 500000UL;

/** number of attempts before giving up on io_setup(). */
static const int OS_AIO_IO_SETUP_RETRY_ATTEMPTS = 5;

/** Insert buffer segment id */
static const ulint IO_IBUF_SEGMENT = 0;

/** Log segment id */
static const ulint IO_LOG_SEGMENT = 1;

extern char *srv_log_group_home_dir;

int innobase_mysql_tmpfile(const char *path);


/** Static declarations */
AIO *AIO::s_reads;
AIO *AIO::s_writes;
AIO *AIO::s_ibuf;
AIO *AIO::s_log;
AIO *AIO::s_sync;

/** Constructor
@param[in]	id		The latch ID
@param[in]	n		Number of AIO slots
@param[in]	segments	Number of segments */
AIO::AIO(latch_id_t id, ulint n, ulint segments)
    : m_slots(n),
      m_n_segments(segments),
      m_n_reserved()
#ifdef LINUX_NATIVE_AIO
      ,
      m_aio_ctx(),
      m_events(m_slots.size())
#elif defined(_WIN32)
      ,
      m_handles()
#endif /* LINUX_NATIVE_AIO */
{
  ut_a(n > 0);
  ut_a(m_n_segments > 0);

  mutex_create(id, &m_mutex);

  m_not_full = os_event_create("aio_not_full");
  m_is_empty = os_event_create("aio_is_empty");

#ifdef LINUX_NATIVE_AIO
  memset(&m_events[0], 0x0, sizeof(m_events[0]) * m_events.size());
#endif /* LINUX_NATIVE_AIO */

  os_event_set(m_is_empty);
}


/** Initialise the slots */
dberr_t AIO::init_slots() {
  for (ulint i = 0; i < m_slots.size(); ++i) {
    Slot &slot = m_slots[i];

    slot.pos = static_cast<uint16_t>(i);

    slot.is_reserved = false;

#ifdef WIN_ASYNC_IO

    slot.handle = CreateEvent(NULL, TRUE, FALSE, NULL);

    OVERLAPPED *over = &slot.control;

    over->hEvent = slot.handle;

    (*m_handles)[i] = over->hEvent;

#elif defined(LINUX_NATIVE_AIO)

    slot.ret = 0;

    slot.n_bytes = 0;

    memset(&slot.control, 0x0, sizeof(slot.control));

#endif /* WIN_ASYNC_IO */
  }

  return (DB_SUCCESS);
}

#ifdef LINUX_NATIVE_AIO
/** Initialise the Linux Native AIO interface */
dberr_t AIO::init_linux_native_aio() {
  /* Initialize the io_context array. One io_context
  per segment in the array. */

  ut_a(m_aio_ctx == NULL);

  m_aio_ctx = static_cast<io_context **>(
      ut_zalloc_nokey(m_n_segments * sizeof(*m_aio_ctx)));

  if (m_aio_ctx == NULL) {
    return (DB_OUT_OF_MEMORY);
  }

  io_context **ctx = m_aio_ctx;
  ulint max_events = slots_per_segment();

  for (ulint i = 0; i < m_n_segments; ++i, ++ctx) {
    if (!linux_create_io_ctx(max_events, ctx)) {
      /* If something bad happened during aio setup
      we should call it a day and return right away.
      We don't care about any leaks because a failure
      to initialize the io subsystem means that the
      server (or atleast the innodb storage engine)
      is not going to startup. */
      return (DB_IO_ERROR);
    }
  }

  return (DB_SUCCESS);
}
#endif /* LINUX_NATIVE_AIO */

/** Count the number of free slots
@return number of reserved slots */
ulint AIO::pending_io_count() const {
  acquire();

#ifdef UNIV_DEBUG
  ut_a(m_n_segments > 0);
  ut_a(!m_slots.empty());

  ulint count = 0;

  for (ulint i = 0; i < m_slots.size(); ++i) {
    const Slot &slot = m_slots[i];

    if (slot.is_reserved) {
      ++count;
      ut_a(slot.len > 0);
    }
  }

  ut_a(m_n_reserved == count);
#endif /* UNIV_DEBUG */

  ulint reserved = m_n_reserved;

  release();

  return (reserved);
}

/** Calculates local segment number and aio array from global segment number.
@param[out]	array		aio wait array
@param[in]	segment		global segment number
@return local segment number within the aio array */
ulint AIO::get_array_and_local_segment(AIO **array, ulint segment) {
  ulint local_segment;
  ulint n_extra_segs = (srv_read_only_mode) ? 0 : 2;

  ut_a(segment < os_aio_n_segments);

  if (!srv_read_only_mode && segment < n_extra_segs) {
    /* We don't support ibuf/log IO during read only mode. */

    if (segment == IO_IBUF_SEGMENT) {
      *array = s_ibuf;

    } else if (segment == IO_LOG_SEGMENT) {
      *array = s_log;

    } else {
      *array = NULL;
    }

    local_segment = 0;

  } else if (segment < s_reads->m_n_segments + n_extra_segs) {
    *array = s_reads;
    local_segment = segment - n_extra_segs;

  } else {
    *array = s_writes;

    local_segment = segment - (s_reads->m_n_segments + n_extra_segs);
  }

  return (local_segment);
}

/** Calculates segment number for a slot.
@param[in]	array		AIO wait array
@param[in]	slot		slot in this array
@return segment number (which is the number used by, for example,
        I/O-handler threads) */
ulint AIO::get_segment_no_from_slot(const AIO *array, const Slot *slot) {
  ulint segment;
  ulint seg_len;

  if (array == s_ibuf) {
    ut_ad(!srv_read_only_mode);

    segment = IO_IBUF_SEGMENT;

  } else if (array == s_log) {
    ut_ad(!srv_read_only_mode);

    segment = IO_LOG_SEGMENT;

  } else if (array == s_reads) {
    seg_len = s_reads->slots_per_segment();

    segment = (srv_read_only_mode ? 0 : 2) + slot->pos / seg_len;
  } else {
    ut_a(array == s_writes);

    seg_len = s_writes->slots_per_segment();

    segment = s_reads->m_n_segments + (srv_read_only_mode ? 0 : 2) +
              slot->pos / seg_len;
  }

  return (segment);
}

/** Frees a slot in the aio array. Assumes caller owns the mutex.
@param[in,out]	slot		Slot to release */
void AIO::release(Slot *slot) {
  ut_ad(is_mutex_owned());

  ut_ad(slot->is_reserved);

  slot->is_reserved = false;

  --m_n_reserved;

  if (m_n_reserved == m_slots.size() - 1) {
    os_event_set(m_not_full);
  }

  if (m_n_reserved == 0) {
    os_event_set(m_is_empty);
  }

#ifdef WIN_ASYNC_IO

  ResetEvent(slot->handle);

#elif defined(LINUX_NATIVE_AIO)

  if (srv_use_native_aio) {
    memset(&slot->control, 0x0, sizeof(slot->control));
    slot->ret = 0;
    slot->n_bytes = 0;
  } else {
    /* These fields should not be used if we are not
    using native AIO. */
    ut_ad(slot->n_bytes == 0);
    ut_ad(slot->ret == 0);
  }

#endif /* WIN_ASYNC_IO */
}


/** Frees a slot in the AIO array. Assumes caller doesn't own the mutex.
@param[in,out]	slot		Slot to release */
void AIO::release_with_mutex(Slot *slot) {
  acquire();

  release(slot);

  release();
}


/** Dispatch an AIO request to the kernel.
@param[in,out]	slot		an already reserved slot
@return true on success. */
bool AIO::linux_dispatch(Slot *slot) {
  ut_a(slot->is_reserved);
  ut_ad(slot->type.validate());

  /* Find out what we are going to work with.
  The iocb struct is directly in the slot.
  The io_context is one per segment. */

  ulint io_ctx_index;
  struct iocb *iocb = &slot->control;

  io_ctx_index = (slot->pos * m_n_segments) / m_slots.size();

  int ret = io_submit(m_aio_ctx[io_ctx_index], 1, &iocb);

  /* io_submit() returns number of successfully queued requests
  or -errno. */

  if (ret != 1) {
    errno = -ret;
  }

  return (ret == 1);
}


/** Creates an io_context for native linux AIO.
@param[in]	max_events	number of events
@param[out]	io_ctx		io_ctx to initialize.
@return true on success. */
bool AIO::linux_create_io_ctx(ulint max_events, io_context_t *io_ctx) {
  ssize_t n_retries = 0;

  for (;;) {
    memset(io_ctx, 0x0, sizeof(*io_ctx));

    /* Initialize the io_ctx. Tell it how many pending
    IO requests this context will handle. */

    int ret = io_setup(max_events, io_ctx);

    if (ret == 0) {
      /* Success. Return now. */
      return (true);
    }

    /* If we hit EAGAIN we'll make a few attempts before failing. */

    switch (ret) {
      case -EAGAIN:
        if (n_retries == 0) {
          /* First time around. */
          ib::warn(ER_IB_MSG_757) << "io_setup() failed with EAGAIN."
                                     " Will make "
                                  << OS_AIO_IO_SETUP_RETRY_ATTEMPTS
                                  << " attempts before giving up.";
        }

        if (n_retries < OS_AIO_IO_SETUP_RETRY_ATTEMPTS) {
          ++n_retries;

          ib::warn(ER_IB_MSG_758) << "io_setup() attempt " << n_retries << ".";

          os_thread_sleep(OS_AIO_IO_SETUP_RETRY_SLEEP);

          continue;
        }

        /* Have tried enough. Better call it a day. */
        ib::error(ER_IB_MSG_759)
            << "io_setup() failed with EAGAIN after "
            << OS_AIO_IO_SETUP_RETRY_ATTEMPTS << " attempts.";
        break;

      case -ENOSYS:
        ib::error(ER_IB_MSG_760) << "Linux Native AIO interface"
                                    " is not supported on this platform. Please"
                                    " check your OS documentation and install"
                                    " appropriate binary of InnoDB.";

        break;

      default:
        ib::error(ER_IB_MSG_761) << "Linux Native AIO setup"
                                 << " returned following error[" << ret << "]";
        break;
    }

    ib::info(ER_IB_MSG_762) << "You can disable Linux Native AIO by"
                               " setting innodb_use_native_aio = 0 in my.cnf";

    break;
  }

  return (false);
}


/** Checks if the system supports native linux aio. On some kernel
versions where native aio is supported it won't work on tmpfs. In such
cases we can't use native aio as it is not possible to mix simulated
and native aio.
@return: true if supported, false otherwise. */
bool AIO::is_linux_native_aio_supported() {
  int fd;
  io_context_t io_ctx;
  char name[1000];

  if (!linux_create_io_ctx(1, &io_ctx)) {
    /* The platform does not support native aio. */

    return (false);

  } else if (!srv_read_only_mode) {
    /* Now check if tmpdir supports native aio ops. */
    fd = innobase_mysql_tmpfile(NULL);

    if (fd < 0) {
      ib::warn(ER_IB_MSG_763) << "Unable to create temp file to check"
                                 " native AIO support.";

      return (false);
    }
  } else {
    ulint dirnamelen = strlen(srv_log_group_home_dir);

    ut_a(dirnamelen < (sizeof name) - 10 - sizeof "ib_logfile");

    memcpy(name, srv_log_group_home_dir, dirnamelen);

    /* Add a path separator if needed. */
    if (dirnamelen && name[dirnamelen - 1] != OS_PATH_SEPARATOR) {
      name[dirnamelen++] = OS_PATH_SEPARATOR;
    }

    strcpy(name + dirnamelen, "ib_logfile0");

    fd = ::open(name, O_RDONLY);

    if (fd == -1) {
      ib::warn(ER_IB_MSG_764) << "Unable to open"
                              << " \"" << name << "\" to check native"
                              << " AIO read support.";

      return (false);
    }
  }

  struct io_event io_event;

  memset(&io_event, 0x0, sizeof(io_event));

  byte *buf = static_cast<byte *>(ut_malloc_nokey(UNIV_PAGE_SIZE * 2));
  byte *ptr = static_cast<byte *>(ut_align(buf, UNIV_PAGE_SIZE));

  struct iocb iocb;

  /* Suppress valgrind warning. */
  memset(buf, 0x00, UNIV_PAGE_SIZE * 2);
  memset(&iocb, 0x0, sizeof(iocb));

  struct iocb *p_iocb = &iocb;

  if (!srv_read_only_mode) {
    io_prep_pwrite(p_iocb, fd, ptr, UNIV_PAGE_SIZE, 0);

  } else {
    ut_a(UNIV_PAGE_SIZE >= 512);
    io_prep_pread(p_iocb, fd, ptr, 512, 0);
  }

  int err = io_submit(io_ctx, 1, &p_iocb);

  if (err >= 1) {
    /* Now collect the submitted IO request. */
    err = io_getevents(io_ctx, 1, 1, &io_event, NULL);
  }

  ut_free(buf);
  close(fd);

  switch (err) {
    case 1:
      return (true);

    case -EINVAL:
    case -ENOSYS:
      ib::error(ER_IB_MSG_765)
          << "Linux Native AIO not supported. You can either"
             " move "
          << (srv_read_only_mode ? name : "tmpdir")
          << " to a file system that supports native"
             " AIO or you can set innodb_use_native_aio to"
             " FALSE to avoid this message.";

      /* fall through. */
    default:
      ib::error(ER_IB_MSG_766) << "Linux Native AIO check on "
                               << (srv_read_only_mode ? name : "tmpdir")
                               << "returned error[" << -err << "]";
  }

  return (false);
}


/** Creates an aio wait array. Note that we return NULL in case of failure.
We don't care about freeing memory here because we assume that a
failure will result in server refusing to start up.
@param[in]	id		Latch ID
@param[in]	n		maximum number of pending AIO operations
                                allowed; n must be divisible by m_n_segments
@param[in]	n_segments	number of segments in the AIO array
@return own: AIO array, NULL on failure */
AIO *AIO::create(latch_id_t id, ulint n, ulint n_segments) {
  if ((n % n_segments)) {
    ib::error(ER_IB_MSG_828) << "Maximum number of AIO operations must be "
                             << "divisible by number of segments";

    return (NULL);
  }

  AIO *array = UT_NEW_NOKEY(AIO(id, n, n_segments));

  if (array != NULL && array->init() != DB_SUCCESS) {
    UT_DELETE(array);

    array = NULL;
  }

  return (array);
}

/** AIO destructor */
AIO::~AIO() {
#ifdef WIN_ASYNC_IO
  for (ulint i = 0; i < m_slots.size(); ++i) {
    CloseHandle(m_slots[i].handle);
  }
#endif /* WIN_ASYNC_IO */

#ifdef _WIN32
  UT_DELETE(m_handles);
#endif /* _WIN32 */

  mutex_destroy(&m_mutex);

  os_event_destroy(m_not_full);
  os_event_destroy(m_is_empty);

#if defined(LINUX_NATIVE_AIO)
  if (srv_use_native_aio) {
    m_events.clear();
    ut_free(m_aio_ctx);
  }
#endif /* LINUX_NATIVE_AIO */

  m_slots.clear();
}



/** Free the AIO arrays */
void AIO::shutdown() {
  UT_DELETE(s_ibuf);
  s_ibuf = NULL;

  UT_DELETE(s_log);
  s_log = NULL;

  UT_DELETE(s_writes);
  s_writes = NULL;

  UT_DELETE(s_sync);
  s_sync = NULL;

  UT_DELETE(s_reads);
  s_reads = NULL;
}

/** Initializes the asynchronous io system. Creates one array each for ibuf
and log i/o. Also creates one array each for read and write where each
array is divided logically into n_readers and n_writers
respectively. The caller must create an i/o handler thread for each
segment in these arrays. This function also creates the sync array.
No i/o handler thread needs to be created for that
@param[in]	n_per_seg	maximum number of pending aio
                                operations allowed per segment
@param[in]	n_readers	number of reader threads
@param[in]	n_writers	number of writer threads
@param[in]	n_slots_sync	number of slots in the sync aio array
@return true if the AIO sub-system was started successfully */
bool AIO::start(ulint n_per_seg, ulint n_readers, ulint n_writers,
                ulint n_slots_sync) {
#if defined(LINUX_NATIVE_AIO)
  /* Check if native aio is supported on this system and tmpfs */
  if (srv_use_native_aio && !is_linux_native_aio_supported()) {
    ib::warn(ER_IB_MSG_829) << "Linux Native AIO disabled.";

    srv_use_native_aio = FALSE;
  }
#endif /* LINUX_NATIVE_AIO */

  srv_reset_io_thread_op_info();

  s_reads =
      create(LATCH_ID_OS_AIO_READ_MUTEX, n_readers * n_per_seg, n_readers);

  if (s_reads == NULL) {
    return (false);
  }

  ulint start = srv_read_only_mode ? 0 : 2;
  ulint n_segs = n_readers + start;

#ifndef UNIV_HOTBACKUP
  /* 0 is the ibuf segment and 1 is the redo log segment. */
  for (ulint i = start; i < n_segs; ++i) {
    ut_a(i < SRV_MAX_N_IO_THREADS);
    srv_io_thread_function[i] = "read thread";
  }
#endif /* !UNIV_HOTBACKUP */

  ulint n_segments = n_readers;

  if (!srv_read_only_mode) {
    s_ibuf = create(LATCH_ID_OS_AIO_IBUF_MUTEX, n_per_seg, 1);

    if (s_ibuf == NULL) {
      return (false);
    }

    ++n_segments;

#ifndef UNIV_HOTBACKUP
    srv_io_thread_function[0] = "insert buffer thread";
#endif /* !UNIV_HOTBACKUP */

    s_log = create(LATCH_ID_OS_AIO_LOG_MUTEX, n_per_seg, 1);

    if (s_log == NULL) {
      return (false);
    }

    ++n_segments;

#ifndef UNIV_HOTBACKUP
    srv_io_thread_function[1] = "log thread";
#endif /* !UNIV_HOTBAKUP */

  } else {
    s_ibuf = s_log = NULL;
  }

  s_writes =
      create(LATCH_ID_OS_AIO_WRITE_MUTEX, n_writers * n_per_seg, n_writers);

  if (s_writes == NULL) {
    return (false);
  }

  n_segments += n_writers;

#ifndef UNIV_HOTBACKUP
  for (ulint i = start + n_readers; i < n_segments; ++i) {
    ut_a(i < SRV_MAX_N_IO_THREADS);
    srv_io_thread_function[i] = "write thread";
  }
#endif /* !UNIV_HOTBACKUP */

  ut_ad(n_segments >= static_cast<ulint>(srv_read_only_mode ? 2 : 4));

  s_sync = create(LATCH_ID_OS_AIO_SYNC_MUTEX, n_slots_sync, 1);

  if (s_sync == NULL) {
    return (false);
  }

  os_aio_n_segments = n_segments;

  os_aio_validate();

  os_aio_segment_wait_events = static_cast<os_event_t *>(
      ut_zalloc_nokey(n_segments * sizeof *os_aio_segment_wait_events));

  if (os_aio_segment_wait_events == NULL) {
    return (false);
  }

  for (ulint i = 0; i < n_segments; ++i) {
    os_aio_segment_wait_events[i] = os_event_create(0);
  }

  os_last_printout = ut_time();

  return (true);
}


/** Initialise the array */
dberr_t AIO::init() {
  ut_a(!m_slots.empty());

#ifdef _WIN32
  ut_a(m_handles == NULL);

  m_handles = UT_NEW_NOKEY(Handles(m_slots.size()));
#endif /* _WIN32 */

  if (srv_use_native_aio) {
#ifdef LINUX_NATIVE_AIO
    dberr_t err = init_linux_native_aio();

    if (err != DB_SUCCESS) {
      return (err);
    }

#endif /* LINUX_NATIVE_AIO */
  }

  return (init_slots());
}

/** Wakes up a simulated aio i/o-handler thread if it has something to do.
@param[in]	global_segment	The number of the segment in the AIO arrays */
void AIO::wake_simulated_handler_thread(ulint global_segment) {
  ut_ad(!srv_use_native_aio);

  AIO *array;
  ulint segment = get_array_and_local_segment(&array, global_segment);

  array->wake_simulated_handler_thread(global_segment, segment);
}

/** Wakes up a simulated AIO I/O-handler thread if it has something to do
for a local segment in the AIO array.
@param[in]	global_segment	The number of the segment in the AIO arrays
@param[in]	segment		The local segment in the AIO array */
void AIO::wake_simulated_handler_thread(ulint global_segment, ulint segment) {
  ut_ad(!srv_use_native_aio);

  ulint n = slots_per_segment();
  ulint offset = segment * n;

  /* Look through n slots after the segment * n'th slot */

  acquire();

  const Slot *slot = at(offset);

  for (ulint i = 0; i < n; ++i, ++slot) {
    if (slot->is_reserved) {
      /* Found an i/o request */

      release();

      os_event_t event;

      event = os_aio_segment_wait_events[global_segment];

      os_event_set(event);

      return;
    }
  }

  release();
}

/** Select the IO slot array
@param[in,out]	type		Type of IO, READ or WRITE
@param[in]	read_only	true if running in read-only mode
@param[in]	aio_mode	IO mode
@return slot array or NULL if invalid mode specified */
AIO *AIO::select_slot_array(IORequest &type, bool read_only,
                            AIO_mode aio_mode) {
  AIO *array;

  ut_ad(type.validate());

  switch (aio_mode) {
    case AIO_mode::NORMAL:

      array = type.is_read() ? AIO::s_reads : AIO::s_writes;
      break;

    case AIO_mode::IBUF:
      ut_ad(type.is_read());

      /* Reduce probability of deadlock bugs in connection with ibuf:
      do not let the ibuf i/o handler sleep */

      type.clear_do_not_wake();

      array = read_only ? AIO::s_reads : AIO::s_ibuf;
      break;

    case AIO_mode::LOG:

      array = read_only ? AIO::s_reads : AIO::s_log;
      break;

    case AIO_mode::SYNC:

      array = AIO::s_sync;
#if defined(LINUX_NATIVE_AIO)
      /* In Linux native AIO we don't use sync IO array. */
      ut_a(!srv_use_native_aio);
#endif /* LINUX_NATIVE_AIO */
      break;

    default:
      ut_error;
  }

  return (array);
}


/** Get the total number of pending IOs
@return the total number of pending IOs */
ulint AIO::total_pending_io_count() {
  ulint count = s_reads->pending_io_count();

  if (s_writes != NULL) {
    count += s_writes->pending_io_count();
  }

  if (s_ibuf != NULL) {
    count += s_ibuf->pending_io_count();
  }

  if (s_log != NULL) {
    count += s_log->pending_io_count();
  }

  if (s_sync != NULL) {
    count += s_sync->pending_io_count();
  }

  return (count);
}

/** Prints pending IO requests per segment of an aio array.
We probably don't need per segment statistics but they can help us
during development phase to see if the IO requests are being
distributed as expected.
@param[in,out]	file		File where to print
@param[in]	segments	Pending IO array */
void AIO::print_segment_info(FILE *file, const ulint *segments) {
  ut_ad(m_n_segments > 0);

  if (m_n_segments > 1) {
    fprintf(file, " [");

    for (ulint i = 0; i < m_n_segments; ++i, ++segments) {
      if (i != 0) {
        fprintf(file, ", ");
      }

      fprintf(file, ULINTPF, *segments);
    }

    fprintf(file, "] ");
  }
}

/** Prints info about the aio array.
@param[in,out]	file		Where to print */
void AIO::print(FILE *file) {
  ulint count = 0;
  ulint n_res_seg[SRV_MAX_N_IO_THREADS];

  mutex_enter(&m_mutex);

  ut_a(!m_slots.empty());
  ut_a(m_n_segments > 0);

  memset(n_res_seg, 0x0, sizeof(n_res_seg));

  for (ulint i = 0; i < m_slots.size(); ++i) {
    Slot &slot = m_slots[i];
    ulint segment = (i * m_n_segments) / m_slots.size();

    if (slot.is_reserved) {
      ++count;

      ++n_res_seg[segment];

      ut_a(slot.len > 0);
    }
  }

  ut_a(m_n_reserved == count);

  print_segment_info(file, n_res_seg);

  mutex_exit(&m_mutex);
}

/** Print all the AIO segments
@param[in,out]	file		Where to print */
void AIO::print_all(FILE *file) {
  s_reads->print(file);

  if (s_writes != NULL) {
    fputs(", aio writes:", file);
    s_writes->print(file);
  }

  if (s_ibuf != NULL) {
    fputs(",\n ibuf aio reads:", file);
    s_ibuf->print(file);
  }

  if (s_log != NULL) {
    fputs(", log i/o's:", file);
    s_log->print(file);
  }

  if (s_sync != NULL) {
    fputs(", sync i/o's:", file);
    s_sync->print(file);
  }
}


#ifdef UNIV_DEBUG

/** Prints all pending IO for the array
@param[in]	file	file where to print */
void AIO::to_file(FILE *file) const {
  acquire();

  fprintf(file, " %lu\n", static_cast<ulong>(m_n_reserved));

  for (ulint i = 0; i < m_slots.size(); ++i) {
    const Slot &slot = m_slots[i];

    if (slot.is_reserved) {
      fprintf(file, "%s IO for %s (offset=" UINT64PF ", size=%lu)\n",
              slot.type.is_read() ? "read" : "write", slot.name, slot.offset,
              slot.len);
    }
  }

  release();
}

/** Print pending IOs for all arrays */
void AIO::print_to_file(FILE *file) {
  fprintf(file, "Pending normal aio reads:");

  s_reads->to_file(file);

  if (s_writes != NULL) {
    fprintf(file, "Pending normal aio writes:");
    s_writes->to_file(file);
  }

  if (s_ibuf != NULL) {
    fprintf(file, "Pending ibuf aio reads:");
    s_ibuf->to_file(file);
  }

  if (s_log != NULL) {
    fprintf(file, "Pending log i/o's:");
    s_log->to_file(file);
  }

  if (s_sync != NULL) {
    fprintf(file, "Pending sync i/o's:");
    s_sync->to_file(file);
  }
}

#endif

/** Requests for a slot in the aio array. If no slot is available, waits until
not_full-event becomes signaled.

@param[in,out]	type		IO context
@param[in,out]	m1		message to be passed along with the AIO
                                operation
@param[in,out]	m2		message to be passed along with the AIO
                                operation
@param[in]	file		file handle
@param[in]	name		name of the file or path as a NUL-terminated
                                string
@param[in,out]	buf		buffer where to read or from which to write
@param[in]	offset		file offset, where to read from or start writing
@param[in]	len		length of the block to read or write
@return pointer to slot */
Slot *AIO::reserve_slot(IORequest &type, fil_node_t *m1, void *m2,
                        pfs_os_file_t file, const char *name, void *buf,
                        os_offset_t offset, ulint len) {
#ifdef WIN_ASYNC_IO
  ut_a((len & 0xFFFFFFFFUL) == len);
#endif /* WIN_ASYNC_IO */

  /* No need of a mutex. Only reading constant fields */
  ulint slots_per_seg;

  ut_ad(type.validate());

  slots_per_seg = slots_per_segment();

  /* We attempt to keep adjacent blocks in the same local
  segment. This can help in merging IO requests when we are
  doing simulated AIO */
  ulint local_seg;

  local_seg = (offset >> (UNIV_PAGE_SIZE_SHIFT + 6)) % m_n_segments;

  for (;;) {
    acquire();

    if (m_n_reserved != m_slots.size()) {
      break;
    }

    release();

    if (!srv_use_native_aio) {
      /* If the handler threads are suspended,
      wake them so that we get more slots */

      os_aio_simulated_wake_handler_threads();
    }

    os_event_wait(m_not_full);
  }

  ulint counter = 0;
  Slot *slot = NULL;

  /* We start our search for an available slot from our preferred
  local segment and do a full scan of the array. We are
  guaranteed to find a slot in full scan. */
  for (ulint i = local_seg * slots_per_seg; counter < m_slots.size();
       ++i, ++counter) {
    i %= m_slots.size();

    slot = at(i);

    if (slot->is_reserved == false) {
      break;
    }
  }

  /* We MUST always be able to get hold of a reserved slot. */
  ut_a(counter < m_slots.size());

  ut_a(slot->is_reserved == false);

  ++m_n_reserved;

  if (m_n_reserved == 1) {
    os_event_reset(m_is_empty);
  }

  if (m_n_reserved == m_slots.size()) {
    os_event_reset(m_not_full);
  }

  slot->is_reserved = true;
  slot->reservation_time = ut_time();
  slot->m1 = m1;
  slot->m2 = m2;
  slot->file = file;
  slot->name = name;
#ifdef _WIN32
  slot->len = static_cast<DWORD>(len);
#else
  slot->len = static_cast<ulint>(len);
#endif /* _WIN32 */
  slot->type = type;
  slot->buf = static_cast<byte *>(buf);
  slot->ptr = slot->buf;
  slot->offset = offset;
  slot->err = DB_SUCCESS;
  slot->original_len = static_cast<uint32>(len);
  slot->io_already_done = false;
  slot->buf_block = NULL;
  slot->encrypt_log_buf = NULL;

  if (srv_use_native_aio && offset > 0 && type.is_write() &&
      type.is_compressed()) {
    ulint compressed_len = len;

    ut_ad(!type.is_log());

    release();

    void *src_buf = slot->buf;
    slot->buf_block = os_file_compress_page(type, src_buf, &compressed_len);

    slot->buf = static_cast<byte *>(src_buf);
    slot->ptr = slot->buf;
#ifdef _WIN32
    slot->len = static_cast<DWORD>(compressed_len);
#else
    slot->len = static_cast<ulint>(compressed_len);
#endif /* _WIN32 */
    slot->skip_punch_hole = !type.punch_hole();

    acquire();
  }

  /* We do encryption after compression, since if we do encryption
  before compression, the encrypted data will cause compression fail
  or low compression rate. */
  if (srv_use_native_aio && offset > 0 && type.is_write() &&
      type.is_encrypted()) {
    ulint encrypted_len = slot->len;
    Block *encrypted_block;
    byte *encrypt_log_buf;

    release();

    void *src_buf = slot->buf;
    if (!type.is_log()) {
      encrypted_block = os_file_encrypt_page(type, src_buf, &encrypted_len);

      if (slot->buf_block != NULL) {
        os_free_block(slot->buf_block);
      }

      slot->buf_block = encrypted_block;
    } else {
      /* Skip encrypt log file header */
      if (offset >= LOG_FILE_HDR_SIZE) {
        encrypted_block =
            os_file_encrypt_log(type, src_buf, encrypt_log_buf, &encrypted_len);

        if (slot->buf_block != NULL) {
          os_free_block(slot->buf_block);
        }

        slot->buf_block = encrypted_block;

        if (slot->encrypt_log_buf != NULL) {
          ut_free(slot->encrypt_log_buf);
        }

        slot->encrypt_log_buf = encrypt_log_buf;
      }
    }

    slot->buf = static_cast<byte *>(src_buf);

    slot->ptr = slot->buf;

#ifdef _WIN32
    slot->len = static_cast<DWORD>(encrypted_len);
#else
    slot->len = static_cast<ulint>(encrypted_len);
#endif /* _WIN32 */

    acquire();
  }

#ifdef WIN_ASYNC_IO
  {
    OVERLAPPED *control;

    control = &slot->control;
    control->Offset = (DWORD)offset & 0xFFFFFFFF;
    control->OffsetHigh = (DWORD)(offset >> 32);

    ResetEvent(slot->handle);
  }
#elif defined(LINUX_NATIVE_AIO)

  /* If we are not using native AIO skip this part. */
  if (srv_use_native_aio) {
    off_t aio_offset;

    /* Check if we are dealing with 64 bit arch.
    If not then make sure that offset fits in 32 bits. */
    aio_offset = (off_t)offset;

    ut_a(sizeof(aio_offset) >= sizeof(offset) ||
         ((os_offset_t)aio_offset) == offset);

    struct iocb *iocb = &slot->control;

    if (type.is_read()) {
      io_prep_pread(iocb, file.m_file, slot->ptr, slot->len, aio_offset);
    } else {
      ut_ad(type.is_write());
      io_prep_pwrite(iocb, file.m_file, slot->ptr, slot->len, aio_offset);
    }

    iocb->data = slot;

    slot->n_bytes = 0;
    slot->ret = 0;
  }
#endif /* LINUX_NATIVE_AIO */

  release();

  return (slot);
}


