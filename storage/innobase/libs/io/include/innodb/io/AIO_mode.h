#pragma once

#include <innodb/univ/univ.h>

/** Modes for aio operations @{ */
enum class AIO_mode : size_t {
  /** Normal asynchronous i/o not for ibuf pages or ibuf bitmap pages */
  NORMAL = 21,

  /**  Asynchronous i/o for ibuf pages or ibuf bitmap pages */
  IBUF = 22,

  /** Asynchronous i/o for the log */
  LOG = 23,

  /** Asynchronous i/o where the calling thread will itself wait for
  the i/o to complete, doing also the job of the i/o-handler thread;
  can be used for any pages, ibuf or non-ibuf.  This is used to save
  CPU time, as we can do with fewer thread switches. Plain synchronous
  I/O is not as good, because it must serialize the file seek and read
  or write, causing a bottleneck for parallelism. */
  SYNC = 24
};
/* @} */
