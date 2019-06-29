#include <innodb/sync_event/os_event_create.h>
#include <innodb/allocator/UT_NEW_NOKEY.h>
#include <innodb/sync_event/os_event.h>

/**
Creates an event semaphore, i.e., a semaphore which may just have two
states: signaled and nonsignaled. The created event is manual reset: it
must be reset explicitly by calling sync_os_reset_event.
@return	the event handle */
os_event_t os_event_create(const char *name) /*!< in: the name of the
                                             event, if NULL the event
                                             is created without a name */
{
  os_event_t ret = (UT_NEW_NOKEY(os_event(name)));
/**
 On SuSE Linux we get spurious EBUSY from pthread_mutex_destroy()
 unless we grab and release the mutex here. Current OS version:
 openSUSE Leap 15.0
 Linux xxx 4.12.14-lp150.12.25-default #1 SMP
 Thu Nov 1 06:14:23 UTC 2018 (3fcf457) x86_64 x86_64 x86_64 GNU/Linux */
#if defined(LINUX_SUSE)
  os_event_reset(ret);
#endif
  return ret;
}
