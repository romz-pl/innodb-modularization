#pragma once

#include <innodb/univ/univ.h>

struct Srv_cpu_usage {
  int n_cpu;
  double utime_abs;
  double stime_abs;
  double utime_pct;
  double stime_pct;
};
