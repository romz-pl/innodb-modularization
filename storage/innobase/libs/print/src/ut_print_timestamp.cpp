#include <innodb/print/ut_print_timestamp.h>

#include <innodb/thread/os_thread_handle.h>

#include <time.h>

/** Prints a timestamp to a file. */
void ut_print_timestamp(FILE *file) /*!< in: file where to print */
{
  auto thread_id = os_thread_handle();

#ifdef _WIN32
  SYSTEMTIME cal_tm;

  GetLocalTime(&cal_tm);

  fprintf(file, "%d-%02d-%02d %02d:%02d:%02d %#llx", (int)cal_tm.wYear,
          (int)cal_tm.wMonth, (int)cal_tm.wDay, (int)cal_tm.wHour,
          (int)cal_tm.wMinute, (int)cal_tm.wSecond, (ulonglong)thread_id);
#else
  struct tm *cal_tm_ptr;
  time_t tm;

  struct tm cal_tm;
  time(&tm);
  localtime_r(&tm, &cal_tm);
  cal_tm_ptr = &cal_tm;
  fprintf(file, "%d-%02d-%02d %02d:%02d:%02d %#llx", cal_tm_ptr->tm_year + 1900,
          cal_tm_ptr->tm_mon + 1, cal_tm_ptr->tm_mday, cal_tm_ptr->tm_hour,
          cal_tm_ptr->tm_min, cal_tm_ptr->tm_sec, (ulonglong)thread_id);
#endif /* _WIN32 */
}
