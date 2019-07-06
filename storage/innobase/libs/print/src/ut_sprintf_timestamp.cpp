#include <innodb/print/ut_sprintf_timestamp.h>

#include <time.h>
#include <stdio.h>

/** Sprintfs a timestamp to a buffer, 13..14 chars plus terminating NUL. */
void ut_sprintf_timestamp(char *buf) /*!< in: buffer where to sprintf */
{
#ifdef _WIN32
  SYSTEMTIME cal_tm;

  GetLocalTime(&cal_tm);

  sprintf(buf, "%02d%02d%02d %2d:%02d:%02d", (int)cal_tm.wYear % 100,
          (int)cal_tm.wMonth, (int)cal_tm.wDay, (int)cal_tm.wHour,
          (int)cal_tm.wMinute, (int)cal_tm.wSecond);
#else
  struct tm *cal_tm_ptr;
  time_t tm;

  struct tm cal_tm;
  time(&tm);
  localtime_r(&tm, &cal_tm);
  cal_tm_ptr = &cal_tm;
  sprintf(buf, "%02d%02d%02d %2d:%02d:%02d", cal_tm_ptr->tm_year % 100,
          cal_tm_ptr->tm_mon + 1, cal_tm_ptr->tm_mday, cal_tm_ptr->tm_hour,
          cal_tm_ptr->tm_min, cal_tm_ptr->tm_sec);
#endif
}
