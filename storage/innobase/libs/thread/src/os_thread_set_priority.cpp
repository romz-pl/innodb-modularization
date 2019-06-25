#include <innodb/thread/os_thread_set_priority.h>
#include <innodb/logger/error.h>
#include <innodb/logger/info.h>

#ifdef UNIV_LINUX
/* include defs for CPU time priority settings */
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#endif /* UNIV_LINUX */

/** Set priority for current thread.
@param[in]	priority	priority intended to set
@retval		true		set as intended
@retval		false		got different priority after attempt to set */
bool os_thread_set_priority(int priority) {
#ifdef UNIV_LINUX
  setpriority(PRIO_PROCESS, (pid_t)syscall(SYS_gettid), priority);

  /* linux might be able to set different setting for each thread */
  return (getpriority(PRIO_PROCESS, (pid_t)syscall(SYS_gettid)) == priority);
#else
  return (false);
#endif /* UNIV_LINUX */
}


/** Set priority for current thread.
@param[in]	priority	priority intended to set
@param[in]	thread_name	name of thread, used for log message */
void os_thread_set_priority(int priority, const char *thread_name) {
#ifdef UNIV_LINUX
  if (os_thread_set_priority(priority)) {
#ifdef UNIV_NO_ERR_MSGS
    ib::info()
#else
    ib::error(ER_IB_MSG_1262)
#endif /* UNIV_NO_ERR_MSGS */
        << thread_name << " priority: " << priority;
  } else {
#ifdef UNIV_NO_ERR_MSGS
    ib::error()
#else
    ib::info(ER_IB_MSG_1268)
#endif /* UNIV_NO_ERR_MSGS */
        << "If the mysqld execution user is authorized," << thread_name
        << " thread priority can be changed."
        << " See the man page of setpriority().";
  }
#endif /* UNIV_LINUX */
}
