#include <innodb/time/ut_win_init_time.h>

#ifdef _WIN32
using time_fn = VOID(WINAPI *)(_Out_ LPFILETIME);
static time_fn ut_get_system_time_as_file_time = GetSystemTimeAsFileTime;

/** NOTE: The Windows epoch starts from 1601/01/01 whereas the Unix
 epoch starts from 1970/1/1. For selection of constant see:
 http://support.microsoft.com/kb/167296/ */
#define WIN_TO_UNIX_DELTA_USEC 11644473600000000LL

/**
Initialise highest available time resolution API on Windows
@return false if all OK else true */
bool ut_win_init_time() {
  HMODULE h = LoadLibrary("kernel32.dll");
  if (h != nullptr) {
    auto pfn = reinterpret_cast<time_fn>(
        GetProcAddress(h, "GetSystemTimePreciseAsFileTime"));
    if (pfn != nullptr) {
      ut_get_system_time_as_file_time = pfn;
    }
    return false;
  }
  DWORD error = GetLastError();
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_NO_ERR_MSGS
  log_errlog(ERROR_LEVEL, ER_WIN_LOAD_LIBRARY_FAILED, "kernel32.dll", error);
#else
  ib::error() << "LoadLibrary(\"kernel32.dll\") failed:"
              << " GetLastError returns " << error;
#endif /* UNIV_NO_ERR_MSGS */
#else  /* !UNIV_HOTBACKUP */
  fprintf(stderr,
          "LoadLibrary(\"kernel32.dll\") failed:"
          " GetLastError returns %lu",
          error);
#endif /* !UNIV_HOTBACKUP */
  return (true);
}
#endif
