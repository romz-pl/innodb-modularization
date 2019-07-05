#include <innodb/ioasync/AIO.h>

#include <innodb/ioasync/os_aio_n_segments.h>
#include <innodb/sync_event/os_event_set.h>
#include <innodb/logger/warn.h>
#include <innodb/logger/info.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_event/os_event_create.h>
#include <innodb/align/ut_align.h>
#include <innodb/sync_mutex/mutex_destroy.h>
#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/ioasync/SRV_MAX_N_IO_THREADS.h>
#include <innodb/ioasync/srv_io_thread_function.h>

/** time to sleep, in microseconds if io_setup() returns EAGAIN. */
static const ulint OS_AIO_IO_SETUP_RETRY_SLEEP = 500000UL;

/** number of attempts before giving up on io_setup(). */
static const int OS_AIO_IO_SETUP_RETRY_ATTEMPTS = 5;

/** Insert buffer segment id */
static const ulint IO_IBUF_SEGMENT = 0;

/** Log segment id */
static const ulint IO_LOG_SEGMENT = 1;

/** Set if InnoDB must operate in read-only mode. We don't do any
recovery and open all tables in RO mode instead of RW mode. We don't
sync the max trx id to disk either. */
extern bool srv_read_only_mode;

/* If this flag is TRUE, then we will use the native aio of the
OS (provided we compiled Innobase with it in), otherwise we will
use simulated aio we build below with threads.
Currently we support native aio on windows and linux */
extern bool srv_use_native_aio;

extern char *srv_log_group_home_dir;

int innobase_mysql_tmpfile(const char *path);
void srv_reset_io_thread_op_info();

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

